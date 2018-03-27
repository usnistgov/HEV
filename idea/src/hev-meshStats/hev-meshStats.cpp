#include <osgDB/ReadFile>
#include <osgUtil/Statistics>

int main(int argc, char* argv[]) {
    if (argc < 1) {
        std::cerr << "usage: " << argv[0] << " [file...]" << std::endl;
        exit(EXIT_SUCCESS);
    }

    for (int i=1; i<argc; ++i) {
        osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(argv[i]);
        osgUtil::StatsVisitor stats;
        node->accept(stats);
        stats.totalUpStats();
        std::cout << argv[i] << " stats:" << std::endl;
        stats.print(std::cout);
        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}

