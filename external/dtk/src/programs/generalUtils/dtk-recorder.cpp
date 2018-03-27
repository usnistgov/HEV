#include <dtk.h>
#include <string>
#include <signal.h>
#include <time.h>

using namespace std;

bool done = false;

void signalHandler( int sig )
{
    if( sig == SIGINT )
    {
        if( done )
            exit(1);
        done = true;
    }
    if( sig == SIGALRM )
    {
        printf( "alarm\n" );
    }
}

void waitToWrite( double t )
{
    struct timespec tv;
    tv.tv_sec = (int)t;
    tv.tv_nsec = (int)(t*1000000000);
    nanosleep(&tv,NULL);
}

/**
    @file dtk-recorder.cpp

    dtk-recorder will record and playback one or more dtk shared memory segments in the order they
    were written to.   It uses queued shared memory which timestamps the write operations. 

    Usage: 
        dtk-recorder --help
        dtk-recorder --record SEGMENTA:SEGMENTB:SEGMENTC --saveto FILE
        dtk-recorder --query FILE
        dtk-recorder --play FILE [--speed SPEED] [--start STARTTIME_S] [--end ENDTIME_S] [--segments SEGMENTA:SEGMENTB]

    If no --segments are specified then all segments in FILE will be played.
    If no --end ENDTIME_S is specified (or ENDTIME_S < STARTTIME_S) then the whole file will play
    If SPEED <= 0 then SPEED will default to real time.
*/


bool findHelpArgument( int argc, char** argv )
{
    for( int i = 1; i < argc; i++ )
        if( std::string(argv[i]) == std::string("--help") )
            return true;
    return false;
}

void printUsage()
{
    printf( "\n    dtk-recorder will record and playback 1 or more dtk shared memory segments in the order they\n" );
    printf( "    were written to.   It uses queued shared memory which timestamps the write operations. \n\n" );

    printf( "    Usage: \n" );
    printf( "        dtk-recorder --help\n" );
    printf( "        dtk-recorder --record SEGMENTA:SEGMENTB:SEGMENTC --saveto FILE\n" );
    printf( "        dtk-recorder --query FILE\n" );
    printf( "        dtk-recorder --play FILE [--speed SPEED] [--start STARTTIME_S] [--end ENDTIME_S] [--segments SEGMENTA:SEGMENTB]\n\n");

    printf("    If no --segments are specified then all segments in FILE will be played, otherwise only the specified segments\n");
    printf("    If no --end ENDTIME_S is specified (or ENDTIME_S < STARTTIME_S) then the whole file will play\n");
    printf("    If SPEED <= 0 then SPEED will default to real time.\n\n");
}

bool isArgumentName( const std::string& name )
{
    if( name == std::string("--record") ||
        name == std::string("--query") ||
        name == std::string("--saveto") ||
        name == std::string("--play") ||
        name == std::string("--speed") ||
        name == std::string("--start") ||
        name == std::string("--segments") ||
        name == std::string("--end") ||
        name == std::string("--help") )
            return true;
    return false;
}

std::string findArgument( int argc, char** argv, const std::string& name )
{
    for( int i = 1; i < argc; i++ )
        if( std::string(argv[i]) == name )
        {
            if( (i+1) >= argc )
            {
                printUsage();
                exit(1);
            }
            std::string rtn(argv[i+1]);
            if( isArgumentName( rtn ) )
            {
                printUsage();
                exit(1);
            }
            return rtn;
        }
    return std::string();
}

std::vector<std::string> split( const std::string& str, char ch )
{
    std::vector<std::string> rtn;
    std::string current;
    for( unsigned int i = 0; i < str.size(); i++ )
    {
        if( str[i] != ch )
        {
            current += str[i];
            continue;
        }
        else
        {
            if( current != std::string() )
                rtn.push_back(current);

            current = std::string();
        }
    }
    if( current != std::string() )
        rtn.push_back(current);

    return rtn;
}

/**
    Records segments to a file of the following format:

    NUM_SEGMENTS SEGMENT0_NAME SEGMENT0_SIZE SEGMENT1_NAME SEGMENT1_SIZE  ... SEGMENT(NUM-1)_NAME 
    SEG_INDEX TIME_SEC DATA[SEGMENT[SEG_INDEX]_SIZE]
    ... 

*/
void recordSegmentsToFile( const std::string& segment_names, const std::string& filename )
{
    printf( "recording shared memory segment(s) %s to file %s\n", segment_names.c_str(), filename.c_str() );    

    std::vector<dtkSharedMem*>::iterator shm_itr;
    double           last_time   = -1;
    double           start_time  = 0;
    unsigned char    num_segments;

    FILE* file = fopen( filename.c_str(), "w" );
    if( !file )
    {
        perror( "unable to open file" );
        return;
    }

    std::vector<std::string> names = split(segment_names, ':' );
    std::vector<std::string>::const_iterator itr = names.begin();
    std::vector<dtkSharedMem*> shm;

    while( itr != names.end() )
    {
        dtkSharedMem* tmp = new dtkSharedMem( itr->c_str() );
        tmp->queue();
        tmp->timeStamp();
        if( !tmp->isValid() )
        {
            fprintf( stderr, "Failed to create shared memory segment \"%s\"\n", itr->c_str() );
            delete tmp;
            goto cleanup;
        }
        if( !tmp->getSize() > 254)
        {
            fprintf( stderr, "Failed to record memory segment \"%s\" because its size is greater than 254 bytes\n", itr->c_str() );
            delete tmp;
            goto cleanup;
        }
        std::string name = tmp->getShortName();
        if( name.size() > 254 )
        {
            fprintf( stderr, "Failed to record memory segment \"%s\" because its name is more than 254 characters long\n", name.c_str() );
            delete tmp;
            goto cleanup;
        }
        shm.push_back( tmp );
        itr++;
    }

    // write the header information to the file
    if( shm.size() > 254 )
    {
        fprintf( stderr, "dtk-recorder only supports recording 1 to 254 segments\n" );
        goto cleanup;
    }

    num_segments = (unsigned char)shm.size();
    fwrite( &num_segments, 1, sizeof(num_segments), file );

    shm_itr = shm.begin();
    while( shm_itr != shm.end() )
    {
        std::string name = (*shm_itr)->getShortName();
        unsigned char len = (unsigned char)name.size();

        fwrite( &len, 1, sizeof(len), file );
        fwrite( name.c_str(), len, sizeof(len), file );
        
        unsigned char shm_size = (unsigned char) (*shm_itr)->getSize();
        fwrite( &shm_size, 1, sizeof(shm_size), file );

        shm_itr++;
    }

    // clear the queues
    char buffer[255];

    shm_itr = shm.begin();
    while( !done && shm_itr != shm.end() )
    {
        while( !done && (*shm_itr)->qRead(buffer) > 0 );

        shm_itr++;
    }

    // start the recording ... 

    printf( "shm.size(): %d\n", (int)shm.size() );
   
    while( !done )
    {
        double           oldest_time = -1;
        unsigned char    oldest_index = 255;

        // find the oldest write
        for( unsigned int i = 0; i < shm.size(); i++ )
        {
            int qsize = shm[i]->qPeek( NULL, 0 );
            if( qsize > 0 )
            {
                if( oldest_time < 0 || shm[i]->getTimeStamp() < oldest_time )
                {
                    oldest_time = shm[i]->getTimeStamp();
                    oldest_index = i;
                }
            }
        }
        if( last_time == -1 )
            start_time = oldest_time;
        
        if( oldest_index < 255 )
        {
            shm[oldest_index]->qRead( buffer );
            double elapsed = oldest_time - start_time;
            fwrite( &elapsed, 1, sizeof( elapsed ), file );
            fwrite( &oldest_index, 1, sizeof( oldest_index ), file );
            fwrite( buffer, 1, shm[oldest_index]->getSize(), file );
            last_time = oldest_time;
        }
        else
        {
            usleep(1000);
        }
        // record the oldest write
   } 

   cleanup:
        shm_itr = shm.begin();
        while( shm_itr != shm.end() )
        {
            delete *shm_itr;
            shm_itr++;
        }
        fclose( file );
}

void queryFile( const std::string& filename )
{
    printf( "file: %s\n", filename.c_str() );    

    std::vector<int> sizes;
    
    FILE* file = fopen( filename.c_str(), "r" );
    if( !file )
    {
        perror( "unable to open file" );
        return;
    }
    unsigned char num_segments;
    fread( &num_segments, 1, sizeof( num_segments ), file );

    printf( "segments: %d\n", (int)num_segments );
    for( unsigned char i = 0; i < num_segments; i++ )
    {
        unsigned char name_len;
        fread( &name_len, 1, sizeof( name_len ), file );

        char* name = new char[name_len];
        fread( name, name_len, sizeof( char ), file );

        std::string seg_name( name, name_len );
        
        unsigned char seg_len;
        fread( &seg_len, 1, sizeof( seg_len ), file );

        sizes.push_back(seg_len);

        printf( "\t%s   %d bytes\n", seg_name.c_str(), (int)seg_len );
    }
    double start_time = 0;
    if( !fread( &start_time, 1, sizeof(start_time), file ) )
    {
        printf( "Length: 0s\n" );
        printf( "Total Updates: 0\n" );
        return;
    }
    int    count = 0;
    double t = start_time;
    char buffer[255];
    while( !done )
    {
        unsigned char idx;
        if( !fread( &idx, 1, sizeof(idx), file ) ) 
        {
            printf( "Length: %fs\n", t - start_time );
            printf( "Total Updates: %d\n", count );
            done = true;
            return;
        }
        if( !fread( buffer, sizes[idx], sizeof(char), file ) )
        {
            printf( "Length: %fs\n", t - start_time );
            printf( "Total Updates: %d\n", count );
            done = true;
            return;
        }
        count++; 
        double test;
        if( !fread( &test, 1, sizeof(t), file ) )
        {
            printf( "Length: %fs\n", t - start_time );
            printf( "Total Updates: %d\n", count );
            done = true;
            return;
        }
        t = test;
    }
}

/**
    @return true if name is found in names
*/
bool findName( const std::vector<std::string>& names, const std::string& name )
{
    std::vector<std::string>::const_iterator itr = names.begin();
    while( itr != names.end() )
    {
        if( *itr == name )
            return true;
        itr++;
    }
    return false;
}

void playFile( const std::string& filename, const std::string& starttime = "0", const std::string& endtime = "", const std::string& speedstr = "1", const std::string& segments = "" )
{
    float speed = 1;
    float start = 0;
    float end = -1;
    if( starttime.size() )
        sscanf(starttime.c_str(), "%f", &start );
    if( endtime.size() )
        sscanf(endtime.c_str(), "%f", &end );
    if( speedstr.size() )
        sscanf(speedstr.c_str(), "%f", &speed );

    if( speed <= 0 )
        speed = 1;

    std::vector<std::string> names = split(segments, ':' );
    std::vector<int> sizes;

    printf( "playing file: %s at %f speed starting at %f seconds\n", filename.c_str(), speed, start );    
    
    std::vector<dtkSharedMem*> shm;
    std::vector<dtkSharedMem*>::iterator shm_itr;
    double start_time = 0;
    int    count = 0;
    double t;
    double wait_time = 0;
    char buffer[255];


    
    FILE* file = fopen( filename.c_str(), "r" );
    if( !file )
    {
        perror( "unable to open file" );
        return;
    }
    unsigned char num_segments;
    fread( &num_segments, 1, sizeof( num_segments ), file );

    for( unsigned char i = 0; i < num_segments; i++ )
    {
        unsigned char name_len;
        fread( &name_len, 1, sizeof( name_len ), file );

        char* name = new char[name_len];
        fread( name, name_len, sizeof( char ), file );

        std::string seg_name( name, name_len );
        
        unsigned char seg_len;
        fread( &seg_len, 1, sizeof( seg_len ), file );

        sizes.push_back(seg_len);

        if( names.size() == 0 )
        {
            dtkSharedMem* tmp = new dtkSharedMem( seg_len, seg_name.c_str() );
            if( !tmp->isValid() )
            {
                delete tmp;
                goto cleanup;
            }
            shm.push_back(tmp);
        }
        else 
        {
            dtkSharedMem* tmp = NULL;
            if( findName( names, seg_name ) )
                tmp = new dtkSharedMem( seg_len, seg_name.c_str() );

            shm.push_back(tmp);
        }
    }

    // play the data 

    if( !fread( &start_time, 1, sizeof(start_time), file ) )
        goto cleanup;

    t = start_time;

    while( !done )
    {
        unsigned char idx;
        if( !fread( &idx, 1, sizeof(idx), file ) ) 
        {
            done = true;
            continue;
        }
        if( !fread( buffer, sizes[idx], sizeof(char), file ) )
        {
            done = true;
            continue;
        }

        if( t >= start && shm[idx] != NULL )
        {
            waitToWrite( wait_time / speed );
            shm[idx]->write( buffer );
        }
        count++; 
        double test;
        if( !fread( &test, 1, sizeof(t), file ) )
        {
            done = true;
            continue;
        }
        
        wait_time = test - t;

        t = test;

        if( end > start && t > end )
        {
            done = true;
        }
    }
    
cleanup:
        shm_itr = shm.begin();
        while( shm_itr != shm.end() )
        {
            delete *shm_itr;
            shm_itr++;
        }
        fclose( file );
    
}

int main( int argc, char** argv )
{
    signal( SIGINT, signalHandler );

    if( findHelpArgument( argc, argv ) )
    {
        printUsage();
        return 0;
    }
    std::string record_segments = findArgument( argc, argv, "--record" );
    std::string play            = findArgument( argc, argv, "--play" );
    std::string query           = findArgument( argc, argv, "--query" );
    std::string saveto          = findArgument( argc, argv, "--saveto" );
    std::string start           = findArgument( argc, argv, "--start" );
    std::string speed           = findArgument( argc, argv, "--speed" );
    std::string end             = findArgument( argc, argv, "--end" );
    std::string segments        = findArgument( argc, argv, "--segments" );

    if( play == record_segments && play == query && play == std::string() )
    {
        printUsage();
        return 1;
    }

    if( record_segments != std::string() )
    {
        recordSegmentsToFile( record_segments, saveto );
        return 0;
    }
    if( query != std::string() )
    {
        queryFile( query );
        return 0;
    }
    if( play != std::string() )
    {
        playFile( play, start, end, speed, segments );
        return 0;
    }

    return 0;
}
