#include <osg/ArgumentParser>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <err.h>

int main(int argc, char* argv[]) {

    //PARSE COMMAND LINE ARGUEMENTS================================================================

    osg::ArgumentParser args(&argc, argv);
    osg::ApplicationUsage* usage = args.getApplicationUsage();
    usage->setApplicationName(args.getApplicationName());
    usage->setDescription("Extracts raw volumes from HydratiCA .bimg files");
    usage->setCommandLineUsage(args.getApplicationName() + 
                               " column_number file1 file2 file3 ...");

    unsigned int help = args.readHelpType();
    if (help > 0) {
        args.getApplicationUsage()->write(std::cerr, help);
        exit(EXIT_SUCCESS);
    }

    if (args.errors()) {
        args.writeErrorMessages(std::cerr);
        exit(EXIT_FAILURE);
    }

    args.reportRemainingOptionsAsUnrecognized();
    if (args.errors()) {
        args.writeErrorMessages(std::cerr);
        exit(EXIT_FAILURE);
    }

    if (args.argc() < 3) errx(EXIT_FAILURE, "incorrect number of arguments");

    int column = atoi(args[1]); //column number

    //loop through each of the filename arguments
    for(int i=2; i<args.argc(); i++) { 

        //open .bimg file
        std::string bimg_file = args[i];
        std::ifstream input;
        input.open(bimg_file.c_str(), std::ios::in | std::ios::binary);
        if(!input.is_open()) {
            errx(EXIT_FAILURE, "cant open %s\n", bimg_file.c_str());
        }

        //read header
        unsigned long int nv, x, y, z; //assume long int is 8 bytes
        input.seekg(16, std::ios::beg); //4 bytes (HYBI) + 4 bytes (unused) + 8 bytes (simulation time)
        input.read((char*)&nv, 8); //number of variables
        input.read((char*)&x, 8); //x dimension
        input.read((char*)&y, 8); //y dimension
        input.read((char*)&z, 8); //z dimension
        input.seekg(2048, std::ios::cur); //2048 bytes (unused)
        input.seekg(128*int(nv), std::ios::cur); //skip variable names

        //read data values
        int numCells = int(x)*int(y)*int(z);
        int numVars = int(nv);
        std::vector<double> bimg_data; //assume double is 8 bytes
        bimg_data.resize(numCells*numVars);
        input.read((char*)&bimg_data.front(), 8*numCells*numVars);
        input.close();


        //extract correct column
        if(column >= numVars) errx(EXIT_FAILURE, "column number out of range");
        std::vector<float> raw_data;
        raw_data.resize(numCells);
        for(int i=0; i<numCells; i++) {
            raw_data[i] = bimg_data[i*numVars + column];
        }

        //save as raw file
        std::string out_file = bimg_file;
        std::ostringstream col_str;
        col_str << column;
        out_file.erase(out_file.end()-5, out_file.end());
        out_file += "_" + col_str.str() + ".raw";
        printf("%s\n", out_file.c_str());
        std::ofstream output(out_file.c_str(), std::ios::out | std::ios::binary);
        output.write((char*)&raw_data.front(), sizeof(float)*numCells);
        output.close();

    }

    return 0; //SUCCESS
}

