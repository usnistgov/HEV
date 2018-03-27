
**********************************************
*                                            *
*              May 21, 2015                  *
*                                            *
**********************************************


Date: Thu, 21 May 2015 17:51:11 -0400
From: Steve Satterfield <steve@nist.gov>
To: Judith Terrill <judith.terrill@nist.gov>,
 Wes Griffin <Wesley.Griffin@nist.gov>
Cc: John Hagedorn <hagedorn@nist.gov>,
 Terence Griffin <terence.griffin@nist.gov>
Subject: Location of rendering test cases (summer students 2015)


I reconsidered locating the rendering test cases under the ive
directory as I suggested. Its a little too buried under admin and
other stuff. So I went with Wes' original suggestion and created the
following:


$HEVROOT/test             will be for this project and for other future HEV
 			  testing projects we might think of
 			  implementing.

$HEVROOT/test/renderTest  will be the location of all rendering test cases.


Under renderTest will be one test case per directory. They will be
named in the form of "testCase_XXXXX", where XXXXX is a descriptive
word(s).


renderTest/testCase_piPoints

         is a test case based on hev-testSuite Example 1.


renderTest/testCase_solidSphereShader

         is a test case based on hev-testSuite Example 8.



Each test case should have a run script named: runTestCase.sh

renderTest/runAllTestCases.sh is a master run script that will run all
                               test cases. When it executes, it looks
                               in all sub-directories and
                               runTestCase.sh script and if found runs
                               it.


I envision the rendering test project to be implemented as three
components:

test/renderTest            where the test cases live and are executed.

hev-testSuite              are the "commands" to exercise HEV software/hardware
 			   generating new images for comparison with
 			   the previously saved base line criteria.

 			   Source located $HEVROOT/idea/src/hev-testSuite
                            and installed in HEV tree with "make install".

hev-testRenderXXXX         are a series of render test commands which are
                            executed and produce a PASSED/FAILED exit code
 			   based upon its implement criteria.

                            Each command in the series is installed
                            independently in the HEV tree with source
                            at $HEVROOT/idea/src/hev-testRenderXXXX. The
                            usual "Make install" procedures are
                            implemented.

I have implemented a skeleton of this structure including all three
components. There are two example test cases implemented.  The
skeleton has not yet (as 5/21/2015 5:30PM) been integrated and release
as a new HEV production version.

However, it is in HEV-beta and with our new implementation of HEV-beta
everyone can review and run the test cases as follows.

Open a fresh shell window with no HEV environment set, then type the
following commands:

    $ hevb

    $ cd $HEVROOT/test/renderTest

    $ ./runAllTestCases.sh

Note, one of the test case is supposed to fail.



The runTestCase.sh script in each test case actually implements the
test case. Think of it as the glue that combines all the components:

    Newly generated images created by hev-testSuite;
    Base line images and reference data stored in the test case directory;
    Evaluation of a passed/failed result implemented by hev-testRenderXXXX.


The example test cases are implemented following this structure.

-Steve




**********************************************
*                                            *
*              June 3, 2015                  *
*                                            *
**********************************************



Additions to $HEVROOT/test/renderTest
-------------------------------------


renderTest/bin              Executable commands useful for the "renderTest"
			    project which are used by multiple test cases.
			    
			    Anything specific to one test case belongs in that
			    test case sub-directory.


renderTest/man/man1         Man pages for commands in bin

renderTest/lib              Any needed libraries

renderTest/src		    All source code for bin

renderTest/GNUmakefile      Use this to build and install
			    Note: a full HEV build will traverse this.

The goal for this structure is to prevent duplication of commands in
multiple test cases, for commands that do not belong in the larger HEV
environment. However, as part of the decision for placing something
here, consider if its useful in the full HEV environment in which
case, it should be installed as a full HEV command.

This bin directory will not in the PATH. Here are some methods for
accessing thecommands them:

1) Absolute path, for example

   $HEVROOT/test/renderTest/bin/compare-binary


2) Relative path, for example in a runTestCase.sh

   ../bin/compare-binary


3) Add to path in runTestCase.sh

   # Put this at the beginning of runTestCase.sh
   export PATH=$HEVROOT/test/renderTest/bin:$PATH

   # Then use in the script as any command
   compare-binary



A note about Git
----------------

   I have added a local .gitignore, $HEVROOT/test/renderTest/.gitignore
   Currently ./bin and ./lib are ignored (they are built from src).


**********************************************
*                                            *
*              June 5, 2015                  *
*                                            *
**********************************************

Date: Fri, 5 Jun 2015 13:56:09 -0400
From: Steve Satterfield <steve@nist.gov>
To: Judith Terrill <judith.terrill@nist.gov>
Cc: Wes Griffin <Wesley.Griffin@nist.gov>,
    John Hagedorn <hagedorn@nist.gov>,
    Terence Griffin <terence.griffin@nist.gov>
Subject: Updates for HEV rendering tests


Judy,

I have setup the infrastructure for the source files part of your HEV
rendering tests. Its been pushed up to the Git repository.  You can
now do:

     get pull

to up date your local HEV development tree. I have made changes to the
hev-testSuite man page with necessary information. Please read that
first.  In particular note the section titled "The BIG Picture".  It
might also be useful to re-read $HEVROOT/test/renderTest/ReadMe.txt.

In the updates you pulled, you will find:

    $HEVROOT/test/renderTest/testCase_shaderProcedure_sgs

its variation of your testCase_shaderProcedure, but changed to conform
to how the scheme is designed. Its  model for additional test cases.

You will notice your source files are gone. They are now in

    $HEVROOT/idea/src/hev-testSuite/test_shaderProcedure_sgs

following the design philosophy described in the "Big Picture".

Also, your scripts buildTestCase.sh and makeTestCase.sh are replaced
by the GNUmakefile in test_shaderProcedure_sgs

By separating the source files from the test case, the build will be
included in the twice daily HEV re-build (HEV Continuous Integration).

If anything is not clear from the documents or example, please ask me.

-Steve

