#include <itkImageFileReader.h>
#include <itkSignedMaurerDistanceMapImageFilter.h>
#include <itkIntensityWindowingImageFilter.h>
#include <itkNthElementPixelAccessor.h>
#include <itkAdaptImageFilter.h>
#include <itkImageToVectorImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkImageFileWriter.h>

#include <itkImageToImageFilter.h>
#include <itkImageRegionIterator.h>

#include <itkImage.h>
#include <itkVector.h>
#include <itkRGBAPixel.h>
#include <itkExceptionObject.h>

#include <string>
#include <iostream>

#include <getopt.h>
#include <stdlib.h>
#include <libgen.h>

using namespace itk;
using namespace std;

typedef Image<RGBAPixel<unsigned char>, 2> TransferType;
typedef VectorImage<float, 3> VectorVolumeType;
typedef VectorVolumeType::PixelType VectorPixelType;
typedef Image<float, 3> ScalarVolumeType;
typedef Image<RGBAPixel<unsigned char>, 3> RGBAVolumeType;
typedef VectorImage<unsigned char, 3> UCharVolumeType;
typedef VectorImage<unsigned short, 3> UShortVolumeType;

////// unary functor that maps input through transfer function
template<class InputPixel, class TransferImage>
class TransferFunctor 
{
public:
    typedef typename TransferImage::Pointer Transfer;
    typedef typename TransferImage::PixelType PixelType;

    TransferFunctor(Transfer ti) : m_ti(ti) {}
    TransferFunctor() {}
    ~TransferFunctor() {}

    // functor is different if transfer function is different
    bool operator!=(const TransferFunctor &t) const {return t.m_ti != m_ti;}
    bool operator==(const TransferFunctor &t) const {return t.m_ti == m_ti;}

    // look up pixel value in transfer function
    PixelType operator()( const InputPixel &p ) const
    {
	typedef Index<2>::IndexValueType I;
	I w = m_ti->GetLargestPossibleRegion().GetSize(0);
	Index<2> i = {{static_cast<I>(w*p[0]),0}};
	return m_ti->GetPixel(i);
    }

private:
    Transfer m_ti;
};

////// filter to find differences in local neighborhood
// sets to PixelType::max() for any pixel that differs from its direct
// or diagonal neighbors differ, and PixelType::Zero for any pixel
// that matches ALL of its neighbor pixels
template <class InputImage, class OutputImage>
class NeighborDifferenceFilter
    : public ImageToImageFilter<InputImage, OutputImage> 
{
public:
    // ITK filter boilerplate
    typedef NeighborDifferenceFilter Self;
    typedef ImageToImageFilter<InputImage, OutputImage> Superclass;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;
    typedef InputImage InputImageType;
    typedef OutputImage OutputImageType;
    typedef typename InputImage::PixelType InputImagePixelType;
    typedef typename OutputImage::PixelType OutputImagePixelType;
    typedef typename OutputImage::RegionType OutputImageRegionType;
    itkNewMacro(Self);
    itkTypeMacro(NeighborDifferenceFilter, ImageToImageFilter);
    itkStaticConstMacro(ImageDimension, unsigned int, 
			InputImage::ImageDimension);
    itkStaticConstMacro(OutputImageDimension, unsigned int, 
			OutputImage::ImageDimension);

#ifdef ITK_USE_CONCEPT_CHECKING
    // concept checks to make sure templated image types do what we need.
    itkConceptMacro(SameDimensionCheck,
	(Concept::SameDimension<ImageDimension, OutputImageDimension>));
    itkConceptMacro(InputEqualityComparisonCheck,
	(Concept::EqualityComparable<InputImagePixelType>));
#endif

    // from on ZeroCrossingImageFilter: implement a version of
    // GenerateInputRequestedRegion() that pads the input region size
    // by the 1-pixel neighborhood
    virtual void GenerateInputRequestedRegion() 
	throw(InvalidRequestedRegionError);

    // process image
    void ThreadedGenerateData(const OutputImageRegionType &outputRegion,
			      int threadId);
};

// pad input region to include neighbors
template <class InputImage, class OutputImage>
void NeighborDifferenceFilter<InputImage, OutputImage>
::GenerateInputRequestedRegion() throw(InvalidRequestedRegionError)
{
    // call the superclass' implementation of this method
    Superclass::GenerateInputRequestedRegion();

    // get pointers to the input and output
    typename Superclass::InputImagePointer inputPtr =
	const_cast<InputImage*>( this->GetInput() );
    typename Superclass::OutputImagePointer outputPtr = this->GetOutput();

    if (! inputPtr || ! outputPtr)
	return;

    // pad a copy of the input requested region
    typename InputImage::RegionType inputRequestedRegion
	= inputPtr->GetRequestedRegion();
    inputRequestedRegion.PadByRadius(1);

    // crop to input
    if (inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion())) {
	inputPtr->SetRequestedRegion(inputRequestedRegion);
	return;
    }
    else {
	// couldn't crop region, throw an exception
	inputPtr->SetRequestedRegion(inputRequestedRegion);
	InvalidRequestedRegionError e(__FILE__, __LINE__);
	e.SetLocation(ITK_LOCATION);
	e.SetDescription("Requested region is (at least partially) outside the largets possible region.");
	e.SetDataObject(inputPtr);
	throw e;
    }
}

template <class InputImage, class OutputImage>
void NeighborDifferenceFilter<InputImage, OutputImage>
::ThreadedGenerateData(const OutputImageRegionType &outputRegion,
		       int threadId)
{
    ProgressReporter progress(this, threadId, 
			      outputRegion.GetNumberOfPixels());

    // input neighborhood iterator
    ConstNeighborhoodIterator<InputImageType> bit;
    typename ConstNeighborhoodIterator<InputImageType>::RadiusType radius;
    radius.Fill(1);

    // output image iterator
    ImageRegionIterator<OutputImageType> it;

    // allocate output
    typename OutputImageType::Pointer output = this->GetOutput();
    typename InputImageType::ConstPointer input = this->GetInput();

    // partition the input into "faces" with differing boundary conditions
    typedef NeighborhoodAlgorithm
	::ImageBoundaryFacesCalculator<InputImageType> FaceCalculator;
    FaceCalculator faceCalculator;
    typename FaceCalculator::FaceListType faceList
	= faceCalculator(input, outputRegion, radius);
    typename FaceCalculator::FaceListType::iterator fit;

    // process regions of the input
    for (fit=faceList.begin(); fit != faceList.end(); ++fit) {
	it = ImageRegionIterator<OutputImageType>(output, *fit);
	bit = ConstNeighborhoodIterator<InputImageType>(radius, input, *fit);
	unsigned int neighborhoodSize = bit.Size();

	// process pixels of the region
	for (bit.GoToBegin(); ! bit.IsAtEnd(); ++it, ++bit) {
	    const InputImagePixelType center = bit.GetCenterPixel();
	    OutputImagePixelType out 
		= NumericTraits<OutputImagePixelType>::Zero;

	    for(unsigned int i=0; i<neighborhoodSize; ++i) {
		if (bit.GetPixel(i) != center) {
		    out = NumericTraits<OutputImagePixelType>::max();
		    break;
		}
	    }
	    it.Set(out);

	    progress.CompletedPixel();
	}
    }
}

void usage(char *prog) {
    cerr << "Usage:\n" << 
prog<<" transferFunction.dat [-r] volume1.mhd [volume2.mhd ...]\n\n"
"Optimizes 3D volume data files given a specific transfer function.\n\n"

"Adds an additional channel to each volume file that tells, at each\n"
"voxel, the distance to the closest voxel with a different transfer\n"
"function value. This can be used to accelerate volume rendering.\n\n"

"The -r options replaces previous optimization data (say, for a new\n"
"transfer function).\n\n"

"transferFunction.dat a color lookup table is the form specified by\n"
"hev-createLUT. If the transfer function file name is '-', previous\n"
"optimization data will be removed from the volume files.\n\n";
}

////// main program
int main(int argc, char *argv[]) {
    int retval = 0;		// return code from program

    bool replace=false;
    struct option longopts[] = {{"replace",0,0,'r'}, {"help",0,0,'h'},{0}};
    char opt;
    while ((opt = getopt_long(argc, argv, "rx", longopts, 0)) != -1) {
	switch(opt) {
	case 'r':
	    replace=true;
	    break;
	default:
	    usage(argv[0]);
	    return 0;
	}
    }

    if (argc-optind < 2) {
	usage(argv[0]);
	return 1;
    }
    char *transferFile = argv[optind];
    if (strcmp(transferFile,"-")==0)
	transferFile = 0;


    // create a directory for temporary files
    char tmpdir[] = "/tmp/volOptimize-XXXXXX"; mkdtemp(tmpdir);

    //////
    // convert transfer function to png and read it in
    string lut = tmpdir; lut += "/transfer.lut";
    string png = tmpdir; png += "/transfer.png";

    try {
	typedef ImageFileReader<TransferType> Transfer;
	Transfer::Pointer transfer = Transfer::New();
	transfer->SetFileName(png.c_str());

	if (transferFile) {
	    string cmd = string("hev-createLUT < ")+transferFile+" > "
		+lut+"&&"+"hev-LUT2png -a "+lut+" "+png;
	    if (system(cmd.c_str())) {
		throw ExceptionObject(
		    transferFile, __LINE__, 
		    string("Error processing transfer function file: ")
		    +transferFile);
	    }
	    transfer->Update();	// do the read, just this once
	}

	//////
	// process files
	for(int i=optind+1; i<argc; ++i) {

	    // Read file
	    const unsigned int dims = 2;
	    typedef ImageFileReader<VectorVolumeType> Reader;
	    Reader::Pointer reader = Reader::New();
	    char *volumeFile = argv[i];
	    reader->SetFileName(volumeFile);

	    // figure out how many channels to copy to output
	    reader->Update();
	    unsigned int channels = reader->GetOutput()->GetVectorLength();
	    if ((replace || !transferFile) && channels > 1) --channels;

	    // Apply transfer function
	    typedef TransferFunctor<VectorPixelType,TransferType> XferFunc;
	    typedef UnaryFunctorImageFilter
		<VectorVolumeType, RGBAVolumeType, XferFunc> Xfer;
	    Xfer::Pointer xfer = Xfer::New();
	    xfer->SetFunctor(XferFunc(transfer->GetOutput()));
	    xfer->SetInput(reader->GetOutput());

	    // find unchanging regions
	    typedef NeighborDifferenceFilter
		<RGBAVolumeType, ScalarVolumeType> Diff;
	    Diff::Pointer diff = Diff::New();
	    diff->SetInput(xfer->GetOutput());

	    // find distance map of edge image
	    typedef SignedMaurerDistanceMapImageFilter 
		<ScalarVolumeType, ScalarVolumeType> DistMap;
	    DistMap::Pointer distmap = DistMap::New();
	    distmap->SetSquaredDistance(false);
	    distmap->SetInput(diff->GetOutput());

	    // rescale to map steps of 0-255 into 0-1 (this scale factor
	    // matches unpacking constant in shader, not data format). Use
	    // window clamping to avoid actually BEING 1, since that's the
	    // OpenGL default for alpha & used to detect unoptimized volumes
	    typedef IntensityWindowingImageFilter
		<ScalarVolumeType, ScalarVolumeType> DistConv;
	    DistConv::Pointer distconv = DistConv::New();
	    distconv->SetWindowMinimum(0); distconv->SetWindowMaximum(254);
	    distconv->SetOutputMinimum(0);
	    distconv->SetOutputMaximum(254./255.);
	    distconv->SetInput(distmap->GetOutput());

	    // extract channels of source image into N scalar volumes
	    typedef AdaptImageFilter<VectorVolumeType, ScalarVolumeType,
		NthElementPixelAccessor <float,VectorPixelType> >
		NthChannel;
	    NthChannel::Pointer ch[channels];
	    for(int j=0; j<channels; ++j) {
		ch[j] = NthChannel::New();
		ch[j]->SetInput(reader->GetOutput());
		ch[j]->GetAccessor().SetElementNumber(j);
	    }

	    // join source image channels with distance map into single image
	    typedef ImageToVectorImageFilter<ScalarVolumeType> Join;
	    Join::Pointer join = Join::New();
	    for(int j=0; j<channels; ++j)
		join->SetInput(j,ch[j]->GetOutput());
	    if (transferFile)
		join->SetInput(channels, distconv->GetOutput());

	    // Filename for output = input with extra ".mha" extension
	    // ensures output (& possible .raw file) in same directory
	    string outvol = string(volumeFile) + ".mha";

	    // Write file, converting to match pixel type from input file
	    switch (reader->GetImageIO()->GetComponentType()) {
	    case ImageIOBase::UCHAR: {
		typedef CastImageFilter
		    <VectorVolumeType, UCharVolumeType> Conv;
		Conv::Pointer conv = Conv::New();
		conv->SetInput(join->GetOutput());

		typedef ImageFileWriter<UCharVolumeType> Writer;
		Writer::Pointer writer = Writer::New();
		writer->SetInput(conv->GetOutput());
		writer->SetFileName(outvol.c_str());
		writer->Update();
		break;
	    }
	    case ImageIOBase::USHORT: {
		typedef CastImageFilter
		    <VectorVolumeType, UShortVolumeType> Conv;
		Conv::Pointer conv = Conv::New();
		conv->SetInput(join->GetOutput());

		typedef ImageFileWriter<UShortVolumeType> Writer;
		Writer::Pointer writer = Writer::New();
		writer->SetInput(conv->GetOutput());
		writer->SetFileName(outvol.c_str());
		writer->Update();
		break;
	    }
	    case ImageIOBase::FLOAT: {
		typedef ImageFileWriter<VectorVolumeType> Writer;
		Writer::Pointer writer = Writer::New();
		writer->SetInput(join->GetOutput());
		writer->SetFileName(outvol.c_str());
		writer->Update();
		break;
	    }
	    default:
		throw ExceptionObject(
		    volumeFile, __LINE__, 
		    string("Unsupported volume input type: ")
		    + reader->GetImageIO()->GetComponentTypeAsString(
			reader->GetImageIO()->GetComponentType()));
	    }

	    // replace the input file
	    rename(outvol.c_str(),volumeFile);
	    cout << i-optind << '/' << argc-optind-1 << ' ' << volumeFile 
		 << "            \r";
	}
	cout << '\n';
    }
    catch (ExceptionObject e) {
	cerr << e.GetDescription() << endl;
	retval = 1;
    }
    catch (...) {
	cerr << "error processing\n";
	retval = 1;
    }

    // clean up transfer function temporary directory
    unlink(png.c_str());
    unlink(lut.c_str());
    rmdir(tmpdir);

    return retval;
}
