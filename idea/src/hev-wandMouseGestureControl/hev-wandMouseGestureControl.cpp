/** Gestures in a 3D Immersive Environment Project 
 
Originally developed by Luis Daniel Catacora
Adapted for IRIS and continued by Divya Mouli 
(With help from Dr. Judith Terrill, Dr. Terence Griffin, and Dr. John Kelso)
NIST      Updated on: 8/10/12

cpp program that recognizes the following gestures:
	-Basic Linear Gestures: 	Up, Right, Down, Left, 
					Up Right, Down Left, Down Right, Down Left
	-Basic Non-Linear Gestures: 	Flick Up, Flick Right, Flick Left
					Throw, Toss,  Wave, Pancake Flip, BackHand Shot
	-Complex Gestures:		Box, Triangle, Circle, Infinity, Loop, Z, X, V, J
Total Number of Gestures: 25

Note: This program recognizes wand gestures based on tracking data collected approximately <= 20 times a second.
User would check the "Gestures" button on the Master Control Panel to enable gesture recognition mode (using the center wand button)
User would then press the Left wand button (button0) at the start of the gesture and release it at the end.

Questions can be directed to danny.catacora@gmail.com and dmouli@andrew.cmu.edu
**/

#include <values.h>
#include <iris.h>
#include <iostream>
#include <fstream>
#include <cmath>

//Array variables 
const int P=20,S=(P-1),A=(S-1);/**array sizes,holds 2.0 sec. of data*/
const int LP=7,LS=(LP-1),LA=(LS-1);
const int SP=4,SS=(SP-1),SA=(SS-1);
const float min_time = .5f, chKP=.0025, chKS=0.0003, chKA=.00003;

//Position Variables, Speed Variables, Accerlation Variables  
float px[P],  py[P],  pz[P],  sx[S],  sy[S],  sz[S],  ax[A],  ay[A],  az[A]; //In the x,y,z directions
float pya[SP],ppi[SP],pro[SP],sya[SS],spi[SS],sro[SS],aya[SA],api[SA],aro[SA]; //Yaw, Pitch, Roll
float x, y, z,yaw, roll, pitch; char f[60];//array of characters for message

//Gesture Counts
int   BoxCount=0, TriCount=0, Zcount=0, Xcount=0, Vcount=0, CircCount=0, InfCount=0, LoopCount=0;//Counts implemented to detect a larger gesture comprised of other gestures 
int   Jcount=0, ThrowCount=0, TossCount=0, WaveCount=0, BHCount=0;

long int count=0;//big numbers needed for counter and message counter

//Smaller Position, Speed, Acceleration Variables 
int   lpX,  lpY,  lpZ,  lsX,  lsY,  lsZ,  laX,  laY,  laZ; 	//Position in x,y,z directions, to assess line gestures
int   spX, spY, spZ, ssX, ssY, ssZ, saX, saY, saZ;        	//Smaller variable for position in x,y,z directions, to assess quick gestures
int   pYA,  pPI,  pRO,  sYA,  sPI,  sRO,  aYA,  aPI,  aRO;	//Constants holding CHANGES in pitch, roll, and yaw
int   spYA, spPI, spRO, ssYA, ssPI,  ssRO,  saYA,  saPI,  saRO;	//Smaller variables for yaw, pitch, and roll, to assess quick angular movements

//Shared Memory Variables 
dtkSharedMem *wandMouse_shm; //Shared mem for mouse 
bool          wandMouseState = false ; //true == wandMouse active
dtkSharedMem *gestureNum;       //Shared mem for gesture number 
dtkSharedMem *wand_shm;         //Shared mem for wand
float         wand_xyzhpr[6];   //Array for xyz and ypr  
dtkSharedMem *button0Shm = NULL;//Shared mem for left wand button (button0) 
dtkSharedMem *button2Shm= NULL;	//Shared mem for right wand button (button2)
dtkSharedMem *GONumber=NULL;
unsigned int  gNum;		//Gesture Number

//Output stream of tracking data for debugging/gesture creation
std::ofstream myfile("zout.txt"); 

//Methods + Functions
int checkAr(float a[], int s, int knum);
void update(float a[], int s, float z);
void set(float a[], int s, float num);
void print(float a[], int s);
void print1(float a[], int s);

//Gestures
bool checkLineLeft();
bool checkLineRight();
bool checkLineUp();
bool checkLineDown();
bool checkDLineUpRight();
bool checkDLineUpLeft();
bool checkDLineDownRight();
bool checkDLineDownLeft();
bool checkFlickUp();
bool checkFlickLeft();
bool checkFlickRight();
bool checkBox();
bool checkTriangle();
bool checkZ();
bool checkX();
bool checkV();
bool checkCircle();
bool checkInfinity();
bool checkLoopdeLoop();
bool checkJ();
bool checkThrow();
bool checkToss();
bool checkWave();
bool checkBHShot();
bool checkPanFlip();

//Other Supporting Motions
bool checkSwing();
bool tiltLeft();
bool tiltRight();
bool stBackhand();
bool stThrow();


///Main method
int main(int argc, char **argv) {
    bool 	  button0=false;
    bool	  button2=false;
    unsigned char old0=0;
    unsigned char old2=0;
    unsigned char gon=0;
    dtkTime t;// dtkTime is a gettimeofday() wrapper
    float acc_t ;
    t.reset(0.f,1);
    acc_t = -MAXFLOAT ;    
    GONumber = new dtkSharedMem("GON"); 

    wand_shm = new dtkSharedMem("wand") ;
    if (!wand_shm || wand_shm->isInvalid())
    {	dtkMsg.add(DTKMSG_ERROR," hev-wandMouseGestureControl:"" couldn't open shared memory segment\"wand\"\n"); return 1;
    }	//ends check if opened wand

    button0Shm=new dtkSharedMem(sizeof(unsigned char),"buttons");
    if (!button0Shm || button0Shm->isInvalid()) 
    {	std::cout<<"couldnt open shared memory"<<std::endl; return 1;
    }	//ends check if opened button0 shared mem

    button2Shm=new dtkSharedMem(sizeof(unsigned char), "buttons");//getenv("SLB_2"));- does not work
    if(!button2Shm || button2Shm->isInvalid())
    {	std::cout<<"couldnt open shared memory"<<std::endl; return 1;
    }	//ends check if opened button2 chared mem

    gestureNum=new dtkSharedMem(sizeof(int), "gestureNum");
    if (!button0Shm || button0Shm->isInvalid())
    	{std::cout<<"couldnt open shared memory"<<std::endl; return 1;
    	}//ends check if opened gesture     

    wandMouse_shm= new dtkSharedMem(sizeof(wandMouseState), "dgl/wandMouse", &wandMouseState); 
    if (!wandMouse_shm || wandMouse_shm->isInvalid()) 
    	{dtkMsg.add(DTKMSG_ERROR," hev-wandMouseGestureControl:"" couldn't open shared memory segment\"dgl/wandMouse\"\n");return 1 ;
    	}//ends if
   
   button0Shm->write(&old0);
   button2Shm->write(&old2);
   
    //Loops 20 times per second
    while (1)
    {   usleep(50000);//sleeps for 50,000 microseconds = .05 seconds
	count++;//temp
	wand_shm->read(wand_xyzhpr);
	button0Shm->read(&old0);
	button2Shm->read(&old2);
	GONumber->read(&gon);
	if((int)old0 == 1) button0 = true; else button0 = false;
        if((int)old2 == 4) button2 = true; else button2 = false;
	x = float(wand_xyzhpr[0]);
        y = float(wand_xyzhpr[1]);
        z = float(wand_xyzhpr[2]);
        yaw = int(wand_xyzhpr[3])%360;
        if (yaw<0) yaw += 360;
        pitch = int(wand_xyzhpr[4])%360;
        if (pitch<0) pitch += 360;
        roll = int(wand_xyzhpr[5])%360;
        if (roll<0) roll += 360;

	if(button0 && (int)gon==1){///Checks if left button is held down to enable gesture recognition AND if user enabled "gestures"
		acc_t += float(t.delta());
		///Adds new data per array for the new point
		update(px,LP,x);               update(py,LP,y);               update(pz,LP,z);			//adds new x,y,z point
		update(sx,LS,(px[1]-px[0]));   update(sy,LS,(py[1]-py[0]));   update(sz,LS,(pz[1]-pz[0]));	//adds new speed
		update(ax,LA,(sx[1]-sx[0]));   update(ay,LA,(sy[1]-sy[0]));   update(az,LA,(sz[1]-sz[0]));	//adds new aceleration
		update(pya,SP,yaw);	       update(ppi,SP,pitch);	      update(pro,SP,roll);		//adds new y,p,r (point)
		update(sya,SS,(pya[1]-pya[0]));update(spi,SS,(ppi[1]-ppi[0]));update(sro,SS,(pro[1]-pro[0]));	//adds speed
		update(aya,SA,(sya[1]-sya[0]));update(api,SA,(spi[1]-spi[0]));update(aro,SA,(sro[1]-sro[0]));	//adds acceleration
		///Sets the variables to signify the overall change
		lpX=checkAr(px,LP,0),  lpY=checkAr(py,LP,0),  lpZ=checkAr(pz,LP,0);	//finds line changes in position
		lsX=checkAr(sx,LS,1),  lsY=checkAr(sy,LS,1),  lsZ=checkAr(sz,LS,1);	//finds line changes in speed
		laX=checkAr(ax,LA,2),  laY=checkAr(ay,LA,2),  laZ=checkAr(az,LA,2);	//finds line changes in acceleration
		spX=checkAr(px,SP,0),  spY=checkAr(py,SP,0),  spZ=checkAr(pz,SP,0);	//finds smaller changes in position
		ssX=checkAr(sx,SS,1),  ssY=checkAr(sy,SS,1),  ssZ=checkAr(sz,SS,1);	//finds smaller changes in speed
		saX=checkAr(ax,SA,2),  saY=checkAr(ay,SA,2),  saZ=checkAr(az,SA,2);	//finds smaller changes in acceleration
		spYA=checkAr(pya,SP,0),spPI=checkAr(ppi,SP,0),spRO=checkAr(pro,SP,0);	//finds small changes in yaw
		ssYA=checkAr(sya,SS,1),ssPI=checkAr(spi,SS,1),ssRO=checkAr(sro,SS,1);	//finds small changes in pitch
		saYA=checkAr(aya,SA,2),saPI=checkAr(api,SA,2),saRO=checkAr(aro,SA,2);	//finds small changes in roll
	
/**
 *		//Debugging mode, that outputs tracking information to zout.txt
 * 		if(myfile){
			myfile<<count;//<<"|X:"<<x<<" Y:"<<y<<" Z:"<<z<<" |YA:"<<yaw<<" PI:"<<pitch<<" RO:"<<roll<<"   |SX:";

			myfile<<"  |px[0]="<<px[0];
			myfile<<"  |py[0]="<<py[0];
			myfile<<"  |pz[0]="<<pz[0];
			myfile<<"  |sx[0]="<<sx[0];
			myfile<<"  |sy[0]="<<sy[0];
           	        myfile<<"  |sz[0]="<<sz[0];
			myfile<<"  |ax[0]="<<ax[0];
              	    	myfile<<"  |ay[0]="<<ay[0];
              	    	myfile<<"  |az[0]="<<az[0]; 
			myfile<<"  pitch="<<pitch;
			myfile<<"  |spi[0]="<<spi[0];
			myfile<<"  |api[0]="<<api[0]; 
			myfile<<"       yaw="<<yaw;
			myfile<<"  |sya[0]="<<sya[0];
			myfile<<"  |aya[0]="<<aya[0]; 
			myfile<<" 	roll="<<roll;
			myfile<<"  |sro[0]="<<sro[0];
			myfile<<"  |aro[0]="<<aro[0];
 
			myfile<<std::endl;}//ends if file is open
		else std::cout<<"Unable to open file"<<std::endl; 
**/
	

		if (roll < 210  && roll > 160){// true if in flip gesture zone
			if (acc_t > min_time){ // have we been in zone long enough to flip? yes
				acc_t = -MAXFLOAT;
				wandMouseState = !wandMouseState; 
				wandMouse_shm->write(&wandMouseState);
				dtkMsg.add(DTKMSG_INFO,"hev-wandMouseGestureControl::postFrame:setting state to %d\n",wandMouseState);
				std::cout<<"Mode Change: Wand Upside Down"<<std::endl;
			//	myfile<<"|Upside Down|";//sprintf(f,"/bin/bash testShell %s %d", "FlipDown", m);system(f);
			}//ends first if
		}//ends bigger if
		else{acc_t = 0.f ;}//ends else
		

/********************** Series of boolean checks that print string to std::out if gesture is recognized **************/

		if(checkLineLeft()){
			std::cout<<"Gesture Recognized: Line Left "<<std::endl;		
			}//ends Line Left 
	
		if(checkLineRight()){
			std::cout<<"Gesture Recognized: Line Right "<<std::endl;
			}//ends Line Right
	
		if(checkLineUp()){
			std::cout<<"Gesture Recognized: Line Up "<<std::endl;
			}//ends Line Up
	
		if(checkLineDown()){
			std::cout<<"Gesture Recognized: Line Down "<<std::endl;
			}//ends Line Down
	
		if(checkBox()){
			std::cout<<"Gesture Recognized: Box"<<std::endl;
			}//ends Box
	
		if(checkFlickUp()){
			std::cout<<"Gesture Recognized: Flick Upwards"<<std::endl;
			}//ends Flick Up

		if(checkFlickLeft()){//
			std::cout<<"Gesture Recognized: Flick Left"<<std::endl;
			}//ends Flick Left
	
		if(checkFlickRight()){
			std::cout<<"Gesture Recognized: Flick Right"<<std::endl;
			}//ends Flick Right
	
		if(checkDLineUpRight()){
			std::cout<<"Gesture Recognized: Diagonal Line Up, Right"<<std::endl;
			}//ends Diagonal Line Up Right 
	
		if(checkDLineUpLeft()){
			std::cout<<"Gesture Recognized: Diagonal Line Up, Left"<<std::endl;
			}//ends Diagonal Line Up Left 
	
		if(checkDLineDownRight()){
			std::cout<<"Gesture Recognized: Diagonal Line Down, Right"<<std::endl;
			}//ends Diagonal Line Down Right 
	
		if(checkDLineDownLeft()){//
			std::cout<<"Gesture Recognized: Diagonal Line Down, Left"<<std::endl;
			}//ends Diagonal Line Down Left 
	
		if(checkTriangle()){
			std::cout<<"Gesture Recognized: Triangle"<<std::endl;
			}//ends Triangle
	
		if(checkZ()){
			std::cout<< "Gesture Recognized : Z" <<std::endl;
                	}//ends Z

		if(checkX()){ 
			std::cout<< "Gesture Recognized : X" <<std::endl;
			}//ends X
	
		if(checkV()) {
			std::cout<< "Gesture Recognized: V" <<std::endl;
			}//ends V

		if(checkCircle()) {
		   	std::cout<< "Gesture Recognized: Circle" <<std::endl;
		   	}//ends Circle

		if(checkInfinity()) {
			std::cout<< "Gesture Recognized: Infinity" <<std::endl;
			}//ends Infinity
	
		if(checkLoopdeLoop()) {
			std::cout<< "Gesture Recognized: Loop de Loop" <<std::endl;
                        }//ends Loop de Loop
		
		if(checkJ()) {
                        std::cout<< "Gesture Recognized: Fish Hook" <<std::endl;
                        }//ends J
		

		if(checkThrow()){
			std::cout<< "Gesture Recognized: Throw" <<std::endl;
			}//ends Throw 

		if(checkToss()){
			std::cout<< "Gesture Recognized: Underhand Toss" << std::endl;
			}//ends Toss 

		if(checkWave()){
			std::cout<< "Gesture Recognized: Wave" <<std::endl;
			}//ends Wave

		if(checkSwing()){
                        //std::cout<< "Motion: Start Underhand Toss" <<std::endl;
                        }//ends Swing

		if(tiltLeft()){
			//std::cout<<"Motion: Waving Left"<<std::endl;
			//std::cout<<"Wave Count="<<WaveCount<<std::endl;;
			}//ends tilt left

		if(tiltRight()){
                        //std::cout<<"Motion: Waving Right"<<std::endl;		                  
			//std::cout<<"Wave Count="<<WaveCount<<std::endl;;
			}//ends tilt right   

		if(stBackhand()){
			//std::cout<<"Motion: Start Backhand"<<std::endl;
			}//ends Start Backhand	
													
		if(stThrow()){
                        //std::cout<<"Motion: Start Throw"<<std::endl;
                	}//ends Start Throw
                

		if(checkBHShot()){
			std::cout<<"Gesture Recognized: Backhand Shot"<<std::endl;
			}//ends Backhand Shot

		if(checkPanFlip()){
			std::cout<<"Gesture Recognized: Pancake Flip"<<std::endl;
			}//ends Pancake Flip

/**********************************************************************************************************************/
	
	}//ends if BUTTON0 is pressed
	  	
	else if (button2 && (int)gon==1){
	//if right button is clicked, will reset gestureNum to unknown gestureNum
		gNum=(unsigned int)30;
		gestureNum->write(&gNum); 	
	}//ends else if	

	
	else {
		//Reset counts of continuous gestures so they must be performed in one click
		//With the exception of "X" 
		TriCount=0; BoxCount=0; Zcount=0; Vcount=0; CircCount=0; InfCount=0;
                LoopCount=0; Jcount=0;  ThrowCount=0; TossCount=0; WaveCount=0; 
		BHCount=0;
		set(px,P,x);set(py,P,y);set(pz,P,z);
		set(pya,SP,yaw);set(ppi,SP,pitch);set(pro,SP,roll);
		
/**	Debugging mode that prints tracking data to zout.txt
		if(myfile){
                        myfile<<count<<"|X:"<<x<<" Y:"<<y<<" Z:"<<z<<" |YA:"<<yaw<<" PI:"<<pitch<<" RO:"<<roll<<" |SX:";
                        myfile<<count<<"off";
                        myfile<<std::endl;}//ends if file is open
                else std::cout<<"Unable to open file"<<std::endl; 
**/


	}//ends else (button is not pressed)

    }//ends while loop

}//ends main method

//This temp method will return a number that defines changes in the array
// 0 = no change , 1 = positive change , 2 = negative change , -1 = undefined change
int checkAr(float a[], int s, int knum){
    float t=a[s-1], K=0.002;
    int chv=0;
    bool zch = true;
	switch(knum){
	case 0:K=chKP;break;
	case 1:K=chKS;break;
	case 2:K=chKA;break;
	}//ends switch case
    for(int c=s-2;c>=0;c--){
	if(t>a[c]&&((a[c]+K)<(t-K)))	 {chv-=1;zch=false;}
     	else if(t<a[c]&&((a[c]+K)>(t-K))){chv+=1;zch=false;}
     	else				  chv+=0;
     	t=a[c];}//ends loop 
    if(chv == (s-1))         return 1;
    else if(chv ==-(s-1))    return 2;
    else if(chv == 0 && zch) return 0;
    else return -1;
}//ends checkAr

/// update array method
void update(float a[], int s, float z){
  float x=0, y=z;
  for(int c=0;c<=s-1;c++){x=a[c];a[c]=y;y=x;}//ends for loop
}//ends method update

///sets the entire array to the number for stable gesture recognition b/c of button implementation
void set(float a[], int s, float num){ 
for(int c=0;c<=s-1;c++) a[c]=num;
}//ends set array method

/**Just prints to file myfile (zout.txt)
void print1(float a[], int s){for(int c=0;c<=s-1;c++) myfile<<a[c]<<" ";//myfile<<ceil(a[c]*100.0)/100.0<<" ";
}//ends print method
**/


/******************************************* Basic Linear Gestures ***************************************************/

//This gesture is a straight line to the left
bool checkLineLeft(){//check for decrease in X with some error acceptance in the y, and z dimensions;
	if(lpX==2 && lpY<=0 && lpZ<=0 && spX==2 && spY<=0 && spZ<=0  
		&& ((yaw>=310 && yaw<=360) || (yaw>=0 && yaw<=40))){
		gNum=(unsigned int)0; 
		gestureNum->write(&gNum);
		if(BoxCount>=3) BoxCount=4; else BoxCount=0; 		//4th line of box
		if(TriCount>=2) TriCount=3; else TriCount=0; 		//3rd line of triangle
		if(CircCount>=4) CircCount=6; else CircCount=0; 	//6th line of circle
		if(InfCount>=6 && InfCount<=8) InfCount=8; 		//8th line of infinity
		if(InfCount>=8) InfCount=10; else InfCount=0; 		//10th line of infinity
		if(LoopCount>=4) LoopCount=6; else LoopCount=0; 	//6th line of loop
		if(Jcount>=1) Jcount=3; else Jcount=0;			//3rd line of fish hook
		Vcount=0; ThrowCount=0;					//Reset all other gestures upon recognition 
		return true;}
	return false;}//ends check for Line Left 

//This gesture is a straight line to the right
bool checkLineRight(){//check for increase in X with some error acceptance in the y and z dimensions
	if(lpZ<=0  && lpX==1  && lpY<=0 && spX==1 && spY<=0 && spZ<=0 
		&& ((yaw>=310 && yaw<=360) || (yaw>=0 && yaw<=40))){
		gNum=(unsigned int)1;
		gestureNum->write(&gNum);
		if(BoxCount>=1) BoxCount=2; else BoxCount=0;		//2nd line of box
		if(TriCount>=2) TriCount=2; else TriCount=0;		//Accounts for human error in drawing Triangle 
		if(Vcount>=1) Vcount=1;					//Accounts for human error in drawing V
		else if (Vcount>=2) Vcount=2; else Vcount=0;		//Accounts for human error in drawing V
		if(CircCount>=0) CircCount=2; else CircCount=0;		//2nd line of circle
		if(Zcount>=0 && Zcount<=1) Zcount=1; 			//1st line of Z
		else if (Zcount>=2) Zcount=3;else Zcount=0;		//3rd line of Z
		if(InfCount>=1 && InfCount<=2) InfCount=2; 		//2nd line of infinity
		if(InfCount>=2) InfCount=4; else InfCount=0;		//4th line of infinity (or 3rd by human error)
		if(LoopCount>=1 && LoopCount<=2) LoopCount=2;		//2nd line of loop
		else if(LoopCount>=8) LoopCount=10; else LoopCount=0;	//10th line of loop (or 9th by human error)
		if(BHCount>=1) BHCount=2; else BHCount=0;		//2nd motion of Backhand Shot
		Jcount=0; 						//Reset all other gestures upon recognition
		return true;}
	return false;}//ends check for Line Right

//This gesture is a straight line up 
bool checkLineUp(){//check for increase in Z with some error acceptance in the x and y dimensions
	if(lpX<=0 && lpY<=0 && lpZ==1 && spX<=0 && spY<=0 && spZ==1 
		&& ((yaw>=310 && yaw<=360) || (yaw>=0 && yaw<=40)) 
		&& !checkFlickUp() ){
		gNum=(unsigned int)2;					
		gestureNum->write(&gNum);
		if(BoxCount>=0) BoxCount=1; else BoxCount=0;		//1st line of box
		if(CircCount>=6) CircCount=8; else CircCount=0;		//8th line of circle (or 7th by human error)
		if(InfCount>=4 && InfCount<=6) InfCount=6;		//6th line of infinity (or 5th by human error)
		else if(InfCount>=10) InfCount=12; else InfCount=0;	//12th line of infinity (or 11th by human error)
		if(LoopCount>=2 && LoopCount<=4) LoopCount=4;		//4th line of infinity (or 3rd by human error)
		else if(LoopCount>=11) LoopCount=11; else LoopCount=0;	//11th line of loop
		if(Jcount>=3) Jcount=5;					//5th line of J (or 4th by human error)
		if(TossCount>=1) TossCount=2; else TossCount=0;		//2nd motion of Toss
		TriCount=0; 						//Reset all other gestures upon recognition
		return true;}
	return false;}//ends check for Line Up


//This gesture is a straight line down
bool checkLineDown(){//check for decrease in Z with some error acceptance in the x and y dimensions
	if(lpX<=0 && lpY<=0 && lpZ==2 && spX<=0 && spY<=0 && spZ==2 
		&& ((yaw>=310 && yaw<=360) || (yaw>=0 && yaw<=40))){
		gNum=(unsigned int)3;
		gestureNum->write(&gNum);
		if(BoxCount>=2) BoxCount=3; else BoxCount=0;		//3rd line of box
		if(CircCount>=2) CircCount=4; 				//4th line of circle (or 3rd by human error)
		else if(Jcount>=0) Jcount=1; else CircCount=Jcount=0;	//1st line of J, accounts for overlap
		if(LoopCount>=6) LoopCount=8; else LoopCount=0;		//8th line in loop (or 7th by human error)
		if(ThrowCount>=1) ThrowCount=2; else ThrowCount=0;	//2nd motion in Throw 
		TriCount=0; Zcount=0; Vcount=0; InfCount=0; 
		TossCount=0;						//Reset all other gestures upon recognition
		return true;}
	return false;}//ends check for Line Down


//This gesture is a diagonal line going up and right
bool checkDLineUpRight(){//check for ++ in X and ++ in Z with some error acceptance in the y dimension
	if(lpX==1 && lpY<=0 && lpZ==1 && spX==1 && spY<=0 && spZ==1 
		&& ((yaw>=310 && yaw<=360) || (yaw>=0 && yaw<=40))){
		gNum=(unsigned int)4;
		gestureNum->write(&gNum);				
		if(TriCount>=0) TriCount=1; else TriCount=0;		//1st line of triangle
		if(Vcount>=1) Vcount=2; else Vcount=0;			//2nd line of V
		if(CircCount>=0 && CircCount<=1) CircCount=1;		//1st line of circle
		else if (CircCount>=8) CircCount=8; else CircCount=0;	//8th line of circle
		if(InfCount>=0 && InfCount<=1) InfCount=1;		//1st line of infinity
		else if(InfCount>=4 && InfCount<=5) InfCount=5; 	//5th line of infinity
		else if(InfCount>=12) InfCount=12; else InfCount=0;	//12th line of infinity
		if(LoopCount>=2 && LoopCount<=3) LoopCount=3;		//3rd line of loop
		else if(LoopCount>=10) LoopCount=11; else LoopCount=0;	//11th line of loop
		if(BHCount>=1) BHCount=2; else BHCount=0;		
		BoxCount=0; Zcount=0; Xcount=0; Jcount=0;		//Reset all other gestures upon recognition
		return true;}
	return false;}//ends check for Line Up-Right


//This gesture is a diagonal line up and left
bool checkDLineUpLeft(){//check for -- in X and ++ in Z with some error acceptance in the y dimension
	if(lpX==2 && lpY<=0 && lpZ==1 && spX==2 && spY<=0 && spZ==1
		& ((yaw>=310 && yaw<=360) || (yaw>=0 && yaw<=40))){
		gNum=(unsigned int)5;
		gestureNum->write(&gNum);
		if(CircCount>=6) CircCount=7; else CircCount=0;		//7th line of circle
		if(InfCount>=6 && InfCount<=7) InfCount=7; 		//7th line of infinity
		else if(InfCount>=10) InfCount=11; else InfCount=0;	//11th line of infinity
		if(LoopCount>=4) LoopCount=5; else LoopCount=0;		//5th line of loop 
		if(Jcount>=3) Jcount=5; else Jcount=0;			//5th line of J (or 4th by human error)
		BoxCount=0; Zcount=0; Xcount=0; Vcount=0; TriCount=0;	//Reset all other gestures upon recognition
		return true;}
	return false;}//ends check for Line Up-Left

//This gesture is a diagonal line going down and right
bool checkDLineDownRight(){//check for ++ in X and -- in Z with some error acceptance in the y dimension
	if(lpX==1 && lpY<=0 && lpZ==2 && spX==1 && spY<=0 && spZ==2
		& ((yaw>=310 && yaw<=360) || (yaw>=0 && yaw<=40))){
		gNum=(unsigned int)6;
		gestureNum->write(&gNum);
		if(TriCount>=1) TriCount=2; else TriCount=0;		//2nd line of Triangle
		if(Xcount>=0) Xcount=1; else Xcount=0;			//1st line of X
		if(CircCount>=2) CircCount=3; else CircCount=0;		//3rd line of circle
		if(InfCount>=2) InfCount=3; else InfCount=0;		//3rd line of infinity
		if(LoopCount>=0 && LoopCount<=1) LoopCount=1;		//1st line of loop
		else if(LoopCount>=8) LoopCount=9; else LoopCount=0;	//9th line of loop
		if(Vcount>=0) Vcount=1; else Vcount=0; 			//1st line of V, accounts for overlap
		if(ThrowCount>=1) ThrowCount=2; else ThrowCount=0;      //2nd motion in Throw
		BoxCount=0; Zcount=0; Jcount=0;				//Reset all other gestures upon recognition
		return true;}
	return false;}//ends check for Line Down-Right

//This gesture is a line going down and left 
bool checkDLineDownLeft(){//check for -- in X and -- in Z with some error acceptance in the y dimension
	if(lpX==2 && lpY<=0 && lpZ==2 && spX==2 && spY<=0 && spZ==2
		& ((yaw>=310 && yaw<=360) || (yaw>=0 && yaw<=40))){
		gNum=(unsigned int) 7;
		gestureNum->write(&gNum);
		if (Zcount>=1) Zcount=2; else Zcount=0;			//2nd line of Z 
		if(Xcount>=1) Xcount=2; else Xcount=0;			//2nd line of X
		if (CircCount>=4) CircCount=5; else CircCount=0;	//5th line of circle
		if(InfCount>=8) InfCount=9; else InfCount=0;		//9th line of infinity
		if(LoopCount>=6) LoopCount=7; else LoopCount=0;		//7th line of loop
		if(Jcount>=1) Jcount=2; else Jcount=0;			//2nd line of J
		if(ThrowCount>=1) ThrowCount=2; else ThrowCount=0;      //2nd motion in Throw
		BoxCount=0; TriCount=0; Vcount=0; 			//Reset all other gestures upon recognition
		return true;}
	return false;}//ends check for Line Down Left


/******************************************** Basic Non-linear Gestures ***********************************************/

//This gesture is a flick upwards, slightly tilting wand towards oneself         
bool checkFlickUp(){	//checks for increase in acceleration and certain angle of wand 
	if( ppi[0]>5 && ppi[0]<60 && (az[0]>.015 || az[1]>.015) && pz[1]<pz[0] ){
		gNum=(unsigned int) 8;
		gestureNum->write(&gNum); 
		BoxCount=0; TriCount=0; Zcount=0; Xcount=0; Vcount=0;
		CircCount=0; InfCount=0; LoopCount=0; Jcount=0;
		//Reset all other counts upon recognition
		return true;}
	return false;}//ends check for Flick up 

//This gesture is a flick to the left
bool checkFlickLeft(){	//checks for increase in yaw angle and increase of 
			//acceleration in x direction 
	if(pya[0]>80 && pya[0]<120 && (ax[0]>.02 || ax[1]>.02)){ 
		gNum=(unsigned int) 9;
		gestureNum->write(&gNum);
		BoxCount=0; TriCount=0; Zcount=0; Xcount=0; Vcount=0;
                CircCount=0; InfCount=0; LoopCount=0; Jcount=0;
          	//Reset all other counts upon recognition
		return true;}
	return false;}//ends check for Flick Left 


//This gesture is a flick to the right
bool checkFlickRight(){	//checks for decrease in yaw angle, and increase of
			//acceleration in x direction
	if(pya[0]>260 && pya[0]<360 && (ax[0]>.02 || ax[1]>.02)){ 
		gNum=(unsigned int) 10;
		gestureNum->write(&gNum);
		BoxCount=0; TriCount=0; Zcount=0; Xcount=0; Vcount=0;
                CircCount=0; InfCount=0; LoopCount=0; Jcount=0;
               	//Reset all other counts upon recognition
		return true;}
	return false;}//ends check for Flick Right


/**This gesture is an overhead throw, beginning with the wand behind the head, tilted upside down,
and ending with a follow-through line downwards**/
bool checkThrow(){
	if(ThrowCount>=2){
		gNum=(unsigned int) 11;
		gestureNum->write(&gNum);
		BoxCount=0; TriCount=0; Zcount=0; Xcount=0; Vcount=0;
                CircCount=0; InfCount=0; LoopCount=0; Jcount=0;
            	//Reset all other counts upon recognition
		return true;}
	return false;} //ends check for Throw 


/**This gesture is an underhand toss, beginning with the wand held behind the user's leg, 
ending with a follow-through line up**/
bool checkToss(){
	if(TossCount>=2 && api[0]<350 && pz[1]<pz[0]){
		gNum=(unsigned int) 12;
		gestureNum->write(&gNum);
		BoxCount=0; TriCount=0; Zcount=0; Xcount=0; Vcount=0;
                CircCount=0; InfCount=0; LoopCount=0; Jcount=0;
		//Reset all other counts upon recognition
       		return true; }
	return false;} //ends check for Underhand Toss

/**This gesture is a wave, with the wand perpendicular to the floor osciallating back and
forth in a semicircle**/
bool checkWave(){
	if(WaveCount>=2){
		gNum=(unsigned int) 22;
		gestureNum->write(&gNum);
		BoxCount=0; TriCount=0; Zcount=0; Xcount=0; Vcount=0;
		CircCount=0; InfCount=0; LoopCount=0; Jcount=0; BHCount=0;
		//Reset all other counts upon recognition 
		return true;}
	return false;}//ends check for wave 

/** This gesture is a right-handed backhand, with the wand starting parallel to the floor, 
buttons facing the back wall, then a steady motion across body**/
bool checkBHShot(){
	if(BHCount>=2){
		gNum=(unsigned int) 23;
		gestureNum->write(&gNum);
		BoxCount=0; TriCount=0; Zcount=0; Xcount=0; Vcount=0;
                CircCount=0; InfCount=0; LoopCount=0; Jcount=0; WaveCount=0;
		//Reset all other counts upon recognition
		return true;}
	return false;}//ends check for Backhand Shot

/**This gesture is a flip of the wand from buttons facing up to down, performed in a steady,
semicircle, right to left motion**/
bool checkPanFlip(){
	if((pitch>335 && pitch<355) && (yaw>55 && yaw<90) && (roll>200 && roll<240)){
		gNum=(unsigned int) 24;
		gestureNum->write(&gNum);
		return true;}
	return false;}//ends check for Pancake Flip


/******************************** Smaller Motions (Helper Functions to Gestures)  **************************************/


bool checkSwing(){//Motion in Underhand Toss
        if( (py[0]>py[1])  &&  (ay[0]>0.025) && (px[0]-px[1]<0.015) && (pz[0]-pz[1]<.015)) {
                if(TossCount>=0) TossCount=1; else TossCount=0;         //1st motion of Toss
                return true;}
        return false;}//ends checkSwing

bool tiltRight(){//Motion in Wave
	if(pitch>15 && pitch<60 && yaw>250 && yaw<285){
		if(WaveCount>=4) WaveCount=4;
		else if(WaveCount>=2) WaveCount=3;
		else if(WaveCount>=0 && WaveCount<=1) WaveCount=1; 
		else WaveCount=0;
		return true;}
	return false;}//ends tiltRight

bool tiltLeft(){//Motion in Wave
	if(pitch>0 && pitch<40 && yaw>80 && yaw<125){
		if(WaveCount>=4) WaveCount=4;
		else if(WaveCount>=3) WaveCount=4;
		else if(WaveCount>=1 && WaveCount<=2) WaveCount=2;
		else WaveCount=0;
		return true;}
	return false;}//ends tiltLeft

bool stBackhand(){//Motion in Backhand Shot
	if(pitch>=340 && pitch<=360 && yaw>=95 && yaw<=130 && roll>=260 && roll<=275){
		if(BHCount>=0) BHCount=1; else BHCount=0;
		return true;}
	return false;}//ends stBackhand

bool stThrow(){//Motion in Throw
	if(pitch>10 && pitch<90 && yaw>180 && yaw<200 && roll>160 && roll<210){
		if(ThrowCount>=0) ThrowCount=1; else ThrowCount=0;
		return true;}
	return false;}//ends stThrow


/********************************* Complex Gestures (Comprised of Basic Gestures) *************************************/

//This gesture is a square, drawn going up, left, down, the right 
bool checkBox(){//check for all four lines in exact order as indicated
	if(BoxCount==4 && lpX<=0 && lpY<=0 && lpZ<=0){
		gNum=(unsigned int)13;
		gestureNum->write(&gNum);
		TriCount=BoxCount=Zcount=Xcount=Vcount=CircCount=0;
		InfCount=LoopCount=0;//Reset all other gestures upon recognition
		return true;}
	return false;}//ends check for Box


//This gesture is a triangle, drawn going up-right, down-right, then left 
bool checkTriangle(){
	if(TriCount==3 && lpX<=0 && lpY<=0 && lpZ<=0 && spX<=0 && spY<=0 && spZ<=0){
		gNum=(unsigned int)14; 
		gestureNum->write(&gNum); 
		BoxCount=0; Zcount=0; Xcount=0; Vcount=0; 
                CircCount=0; InfCount=0; LoopCount=0; 
		Jcount=0;//Reset all other gestures upon recognition
		return true;}
	return false;}//ends check for Triangle


//This gesture is a "Z" motion 
bool checkZ() {
	if ( Zcount==3 && (lpX<=0 && lpY<=0 && lpZ<=0) ) {
		gNum=(unsigned int) 15;
		gestureNum->write(&gNum);
		TriCount=0; BoxCount=0; Jcount=0; Xcount=0; Vcount=0;
                CircCount=0; InfCount=0; LoopCount=0;
  	    	//Reset all other gestures upon recognition
		return true;}
	return false;
	}//ends check for Z 


//This gesture is an "X" motion, down-right, then down-left
bool checkX() {
	if (Xcount==2 && (lpX<=0 && lpY<=0 && lpZ<=0) ) {
		gNum=(unsigned int) 16;
		gestureNum->write(&gNum);
		TriCount=0; BoxCount=0; Zcount=0; Jcount=0; Vcount=0;
                CircCount=0; InfCount=0; LoopCount=0; 
	     	//Reset all other gestures upon recognition
		return true;}
	return false; }// ends check for X


//This gesture is a "V" motion, drawn down-right then up-right 
bool checkV() {
	if (Vcount==2 && (lpX<=0 && lpY<=0 && lpZ<=0) && LoopCount<9){
		gNum= (unsigned int) 17;
		gestureNum->write(&gNum);
		TriCount=0; BoxCount=0; Zcount=0; Xcount=0; Jcount=0;
                CircCount=0; InfCount=0; LoopCount=0;
              	 //Reset all other gestures upon recognition
		return true;}
	return false;}//ends check for V


//This gesture is a circle, drawn clockwise beginning at 9 o'clock 
bool checkCircle() {
	if (CircCount>=8 && (lpX<=0 && lpY<=0 && lpZ<=0 && lsX<=0 && lsY<=0 && lsZ<=0)){
		gNum=(unsigned int) 18;
		gestureNum->write(&gNum);
		TriCount=0; BoxCount=0; Zcount=0; Xcount=0; Vcount=0;
                Jcount=0; InfCount=0; LoopCount=0; 
            	//Reset all other gestures upon recognition
		return true;}
	return false;}//ends check for Circle


//This gesture is an infinity motion, drawn left to right starting at 9 o'clock
bool checkInfinity() {
	if (InfCount>=12 && (lpX<=0 && lpY<=0 && lpZ<=0 && lsX<=0 && lsY<=0 && lsZ<=0)){
		gNum=(unsigned int) 19;
		gestureNum->write(&gNum);
		TriCount=0; BoxCount=0; Zcount=0; Xcount=0; Vcount=0;
                CircCount=0; Jcount=0; LoopCount=0; 
          	//Reset all other gestures upon recognition
		return true;}
	return false;}//ends check for Infinity


//This gesture is a loop, drawn left to right
//Begin going down, then loop upwards
bool checkLoopdeLoop(){
	if (LoopCount>=11 && (lpX<=0 && lpY<=0 && lpZ<=0 && lsX<=0 && lsY<=0 && lsZ<=0)){
		gNum=(unsigned int) 20;
                gestureNum->write(&gNum);
                TriCount=0; BoxCount=0; Zcount=0; Xcount=0; Vcount=0;
                CircCount=0; InfCount=0; Jcount=0;
  		 //Reset all other gestures upon recognition
                return true;}
        return false;}//ends check for Loop de Loop


//This gesture is a "J" motion, drawn from the top, down
bool checkJ(){//"Fish Hook" 
        if (Jcount>=5 && (lpX<=0 && lpY<=0 && lpZ<=0)){
                gNum=(unsigned int) 21;
                gestureNum->write(&gNum);
                TriCount=0; BoxCount=0; Zcount=0; Xcount=0; Vcount=0; 
		CircCount=0; InfCount=0; LoopCount=0; 
		//Reset all other gestures upon recognition
                return true;}
        return false;}    //ends check for J/Fish Hook 
