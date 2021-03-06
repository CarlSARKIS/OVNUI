// ConsoleApplication2.cpp : d�finit le point d'entr�e pour l'application console.
//
#include "stdafx.h"

# define M_PI           3.14159265358979323846

/******************************************************************************\
* Copyright (C) 2012-2014 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#include <iostream>
#include <string.h>
#include <conio.h> // WINDOWS SPECIFIC //
#include "Leap.h"
#include "Ouvriere.h"

using namespace Leap;
using namespace std;

static char escape = (char)27; // code of the "escape" char
bool plop2 = true;
enum GestureState { WaitState = 0, SelectionState, RotationState, TranslationState, ZoomState, EndTransformationState, SelectMenu, DeselectMenu, DeselectionMenu };
int currentState;

class SampleListener : public Listener {
public:
	virtual void onInit(const Controller&);
	virtual void onConnect(const Controller&);
	virtual void onDisconnect(const Controller&);
	virtual void onExit(const Controller&);
	virtual void onFrame(const Controller&);
	virtual void onFocusGained(const Controller&);
	virtual void onFocusLost(const Controller&);
	virtual void onDeviceChange(const Controller&);
	virtual void onServiceConnect(const Controller&);
	virtual void onServiceDisconnect(const Controller&);
private:
	Ouvriere sender;
	bool sendingNeeded;
};

const std::string fingerNames[] = { "Thumb", "Index", "Middle", "Ring", "Pinky" };
const std::string boneNames[] = { "Metacarpal", "Proximal", "Middle", "Distal" };
const std::string stateNames[] = { "STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END" };
bool menuSelected;
void SampleListener::onInit(const Controller& controller) {
	std::cout << "Initialized" << std::endl;
	wstring ip;
	//cout << "Please enter the phone IP adress :" << endl;
	//wcin >> ip;
	//ip = L"192.168.137." + ip;
	//wcout << "Connecting to " << ip << "..." << endl;
	//sender.initConnection(ip);
	//wcout << "Successfully connected to " << ip << " !" << endl;
	//sender.sendData("Message=Connected to the computer !");
	sendingNeeded = true;
	currentState = 0;
}

void SampleListener::onConnect(const Controller& controller) {
	std::cout << "Connected" << std::endl;
	controller.enableGesture(Gesture::TYPE_CIRCLE);
	controller.enableGesture(Gesture::TYPE_KEY_TAP);
	controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
	controller.enableGesture(Gesture::TYPE_SWIPE);
}

void SampleListener::onDisconnect(const Controller& controller) {
	// Note: not dispatched when running in a debugger.
	std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
	std::cout << "Exited" << std::endl;
}
int nbFrameCatch = 0, nbFrameUncatch = 0, nbFrameDeselect = 0;
Hand initHand1, initHand2, hand1, hand2;
Frame previousFrame, startFrame, secondFrame, startFrameMenu, deselectFrame;
float rotationOfHand1x, rotationOfHand1y, rotationOfHand1z, rotationOfHand2x, rotationOfHand2y, rotationOfHand2z;
float initHand1x, initHand2x, initHand1y, initHand2y, initHand1z, initHand2z;
Vector rotationAxis;
int64_t referenceTimeStamp = 0;
int64_t endReferenceTimeStamp = 0;
bool sameHand = true, pointingLeftHand = false, pointingRightHand=false;
Hand leftHandFirstSelect, rightHandFirstSelect;
void SampleListener::onFrame(const Controller& controller) {
	// Get the most recent frame and report some basic information

	std::stringstream ss_data; // data send to the phone

	const Frame frame = controller.frame();


	if (!referenceTimeStamp) {
		referenceTimeStamp = frame.timestamp();
	}
	if (!endReferenceTimeStamp) {
		endReferenceTimeStamp = frame.timestamp();
	}

	//rotationOfHand1 = hand1.rotationAngle(previousFrame);
	//previousFrame = frame;
	controller.config().setFloat("Gesture.Swipe.MinVelocity", 100);
	ss_data << "frame=" << frame.id();
	ss_data << "&time=" << (double)(frame.timestamp()) / 1000000.0;
	/*std::cout << "Frame id: " << frame.id()
	<< ", timestamp: " << frame.timestamp()
	<< ", hands: " << frame.hands().count()
	<< ", fingers: " << frame.fingers().count()
	<< ", tools: " << frame.tools().count()
	<< ", : " << frame.gestures().count() << std::endl;*/
	bool closedLeftFist = false;
	bool closedRightFist = false;
	bool mainDroiteOuverte = false;
	bool mainGaucheOuverte = false;
	bool mainDroiteBas = false;
	bool mainGaucheBas = false;
	float thumbPositionX;
	float ringPositionX;
	float distanceHands;
	float newDistanceHands;


	bool handL = false, handR = false;
	Vector meanDirectionFingers(0,0,0);
	Vector thumbDirection (0, 0, 0);
	HandList hands = frame.hands();
	//std::cout << frame.hands().count() << std::endl;
	int nbFingersFront = 0;
	for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
		
		int comp = 0;
		//	closedLeftFist = false;
		//	closedRightFist = false;
		// Get the first hand
		const Hand hand = *hl;
		std::string handType = hand.isLeft() ? "Left_hand" : "Right_hand";
		//std::cout << std::string(2, ' ') << handType << ", id: " << hand.id()
		//	<< ", palm position: " << hand.palmPosition() << std::endl;
		ss_data << "&" << handType << "=" << hand.palmPosition().x << "/" << hand.palmPosition().y << "/" << hand.palmPosition().z;






		if (hand.isLeft()) {
			ss_data << "&lx=" << hand.palmPosition().x << "&ly=" << hand.palmPosition().y << "&lz=" << hand.palmPosition().z;
			ss_data << "&lHandDirectionx=" << hand.direction().x << "&lHandDirectiony=" << hand.direction().y << "&lHandDirectionz=" << hand.direction().z;
			ss_data << "&lpalmNormalx=" << hand.palmNormal().x << "&lpalmNormaly=" << hand.palmNormal().y << "&lpalmNormalz=" << hand.palmNormal().z;
			
			handL = true;
		}
		else {
			ss_data << "&rx=" << hand.palmPosition().x << "&ry=" << hand.palmPosition().y << "&rz=" << hand.palmPosition().z;
			ss_data << "&rHandDirectionx=" << hand.direction().x << "&rHandDirectiony=" << hand.direction().y << "&rHandDirectionz=" << hand.direction().z;
			ss_data << "&rpalmNormalx=" << hand.palmNormal().x << "&rpalmNormaly=" << hand.palmNormal().y << "rlpalmNormalz=" << hand.palmNormal().z;

			handR = true;
		}

		// Get the hand's normal vector and direction
		const Vector normal = hand.palmNormal();
		const Vector direction = hand.direction();
		//cout << normal.dot(direction)<< endl;
		//std::cout << normal << std::endl;
		// Calculate the hand's pitch, roll, and yaw angles
		/*std::cout << std::string(2, ' ') <<  "pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
		<< "roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
		<< "yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" << std::endl;*/

		// Get the Arm bone
		Arm arm = hand.arm();
		/*std::cout << std::string(2, ' ') <<  "Arm direction: " << arm.direction()
		<< " wrist position: " << arm.wristPosition()
		<< " elbow position: " << arm.elbowPosition() << std::endl;*/

		// Get fingers
		const FingerList fingers = hand.fingers();
		
		float proximalYMin = 1000;
		float proximalYMax = 0;







		for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
			const Finger finger = *fl;
			/*	std::cout << std::string(4, ' ') <<  fingerNames[finger.type()]
			<< " finger, id: " << finger.id()
			<< ", length: " << finger.length()
			<< "mm, width: " << finger.width() << std::endl;*/

			// Get finger bones
			if (fingerNames[finger.type()] != "Thumb") {
				meanDirectionFingers += finger.direction();
			}
			else
				thumbDirection = finger.direction();
			for (int b = 0; b < 4; ++b) {
				Bone::Type boneType = static_cast<Bone::Type>(b);
				Bone bone = finger.bone(boneType);

				// Condition for detecting the closed fists : if comp == 4, one fist is closed ; if comp == 8, both fists are closed.
				if (((fingerNames[finger.type()] != "Thumb") && (((boneNames[boneType] == "Proximal") && (bone.direction().y >= 0.7)) || ((boneNames[boneType] == "Middle") && (bone.direction().z <= -0.5)))))
					comp++;

				// Detecting the X position of the thumb and the ring finger, in order to see if the hand's palm is facing the ground or the sky
				if ((fingerNames[finger.type()] == "Thumb"))
					thumbPositionX = bone.prevJoint().x;
				else if ((fingerNames[finger.type()] == "Ring"))
					ringPositionX = bone.prevJoint().x;

				// Detecting the number of fingers facing front
			//	if (((fingerNames[finger.type()] != "Thumb") && (boneNames[boneType] == "Proximal")))
				//	cout << "FingersFront      " << fingerNames[finger.type()] << "    "  << abs(bone.direction().dot(hand.palmNormal())) << endl;
				//if (hand.isLeft() && (fingerNames[finger.type()] != "Thumb") && (boneNames[boneType] == "Proximal"))
				//	cout << abs(bone.direction().dot(hand.palmNormal())) << endl;
			//	if (((fingerNames[finger.type()] != "Thumb") && (boneNames[boneType] == "Proximal") && (abs(bone.direction().dot(hand.palmNormal())) <= 0.5)))
			//		nbFingersFront++;
		//		cout << fingerNames[finger.type()] << "   " << (boneNames[boneType]) << "  " <<finger.direction() << endl;
				
				
				if (((fingerNames[finger.type()] == "Index") && (boneNames[boneType] == "Proximal") && finger.direction().z < -0.8 ) ||
					((fingerNames[finger.type()] != "Index") && (fingerNames[finger.type()] != "Thumb") && (boneNames[boneType] == "Proximal") && finger.direction().y < 0))
					nbFingersFront++;

		/*		if ((nbFingersFront == 4) && sameHand && hand.isLeft()) {
					cout << "HAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa" << endl;
					pointingLeftHand = true;
				}
				if ((nbFingersFront == 4) && sameHand && hand.isRight())
					pointingRightHand = true;
				if ((nbFingersFront == 8)) {
					cout << "HAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa" << endl;

					pointingLeftHand = true;
					pointingRightHand = true;
				}*/




				if (boneNames[boneType] == "Proximal") {
					//std::cout << fingerNames[finger.type()]
					//<< " finger "  << " direction " << bone.center() << std::endl;
					if (bone.center().y < proximalYMin)
						proximalYMin = bone.center().y;
					if (bone.center().y > proximalYMax)
						proximalYMax = bone.center().y;
				}
				//	std::cout << direction.y << "   " << hand.palmPosition().y<< std::endl;
				/*	if (normal.x <-0.8 && hand.palmPosition().x >70)
					{
					std::cout << "Je vais translater vers la gauche" << std::endl;
					}
					if (normal.x >0.8 && hand.palmPosition().x <-70)
					{
					std::cout << "Je vais translater vers la droite" << std::endl;
					}
					if (normal.y <-0.8 && hand.palmPosition().y >70)
					{
					std::cout << "Je vais translater vers le bas" << std::endl;
					}
					if (normal.y > 0.8 && hand.palmPosition().y < - 70)
					{
					std::cout << "Je vais translater vers le haut" << std::endl;
					}
					if (normal.z <-0.8 && hand.palmPosition().z >70)
					{
					std::cout << "Je vais translater vers le fond" << std::endl;
					}
					if (normal.z >0.8 && hand.palmPosition().z < -70)
					{
					std::cout << "Je vais translater vers le devant" << std::endl;
					}*/

				/*if ((boneNames[boneType] == "Proximal") && (fingerNames[finger.type()] != "Thumb"))
				std::cout << std::string(6, ' ') <<  boneNames[boneType] << ", direction: " << bone.direction().y << std::endl; */
			}
		}
		meanDirectionFingers = meanDirectionFingers / 4;
		//	std::cout << handType << "   " << normal.y << "   " << hand.palmPosition().y << "  " << nbFingersFront << std::endl;

		//if ((hand.isLeft() && (hand.palmNormal().x > 0.8 && hand.palmPosition().x < -70) && (nbFingersFront == 4))
		//	|| (!(hand.isLeft()) && (hand.palmNormal().x < -0.8 && hand.palmPosition().x >70) && (nbFingersFront == 4))){
		//	//std::cout << "Main droite ouverte." << std::endl;
		//	return true;
		//}

	//	if (nbFingersFront)
		//cout << "NB FINGERS FRONT  " << nbFingersFront << endl;

	//	if (nbFingersFront ==8)
		if (hand.isLeft())
			hand1 = hand;
		if (hand.isRight())
			hand2 = hand;


		if (hand.isLeft() && (nbFingersFront >= 2)) {
		//	std::cout << "Main gauche ouverte." << std::endl;
			mainGaucheOuverte = true;
		}
		else if (!(hand.isLeft()) && (nbFingersFront >= 2)){
		//	std::cout << "Main droite ouverte." << std::endl;
			mainDroiteOuverte = true;
		}
		if (hand.isLeft() && (normal.y < -0.97)) {
			//	std::cout << "Main gauche bas." << std::endl;
			mainGaucheBas = true;
		}
		else if (!(hand.isLeft()) && (normal.y < -0.97)){
			//std::cout << "Main droite ouverte." << std::endl;
			mainDroiteBas = true;
		}
		//nbFingersFront = 0;
		proximalYMin = 1000;
		proximalYMax = 0;
		//	else if ((handType == "Right hand") && (thumbPositionX > ringPositionX) && ()

		if (comp == 4) {
			//std::cout << "La " << handType << " est un poing ferm�." << std::endl;
			if (handType == "Left hand")
				closedLeftFist = true;
			else
				closedRightFist = true;
		}
		else if (comp == 8) {
			closedLeftFist = true;
			closedRightFist = true;
		}
		

		comp = 0;
		//	if (closedLeftFist && closedLeftFist)
		//		std::cout << "Deux poings ferm�s." << std::endl;
		if (hand.isLeft()) {
			ss_data << "&lmeanDirectionFingersx=" << meanDirectionFingers.x << "&lmeanDirectionFingersy=" << meanDirectionFingers.y << "&lmeanDirectionFingersz=" << meanDirectionFingers.z;
			ss_data << "&lthumbDirectionx=" << thumbDirection.x << "&lthumbDirectiony=" << thumbDirection.y << "&lthumbDirectionz=" << thumbDirection.z;
		}
		else {
			ss_data << "&rmeanPositionFingersx=" << meanDirectionFingers.x << "&rmeanPositionFingersy=" << meanDirectionFingers.y << "&rmeanPositionFingersz=" << meanDirectionFingers.z;
			ss_data << "&rthumbDirectionx=" << thumbDirection.x << "&rthumbDirectiony=" << thumbDirection.y << "&rthumbDirectionz=" << thumbDirection.z;
		}

		meanDirectionFingers = Vector(0, 0, 0);
		sameHand = false;
	}
	sameHand = true;
	Bone::Type boneTypeD = static_cast<Bone::Type>(3);
//	if (nbFingersFront == 8)
//		cout << " --------- " << abs(frame.hands()[0].fingers()[1].bone(boneTypeD).center().x - frame.hands()[1].fingers()[1].bone(boneTypeD).center().x) << endl;
	bool displayMenu = (nbFingersFront == 8 && abs(frame.hands()[0].fingers()[1].bone(boneTypeD).center().x - frame.hands()[1].fingers()[1].bone(boneTypeD).center().x) < 20);
	//cout << "BOUUUH " << endl;
	//menuSelected = false;


	if (handL){
		ss_data << "&handL=True";
	}
	else {
		ss_data << "&handL=False";
	}
	if (handR){
		ss_data << "&handR=True";
	}
	else {
		ss_data << "&handR=False";
	}

	//if (!(frame.hands()[0].palmNormal().dot(frame.hands()[1].palmNormal()) < -0.8))
	//	cout << "mains plus en face de l'autre" << endl;
	
	// bool selection = mainGaucheOuverte && mainDroiteOuverte && ......... 
	bool selecting = (frame.hands()[0].palmNormal().dot(frame.hands()[1].palmNormal()) < -0.6);
	
	if (selecting){
		//cout << "Hands selecting" << endl;

		endReferenceTimeStamp = frame.timestamp();
	}
	else {
		referenceTimeStamp = frame.timestamp();
		//std::cout << "Hands deselecting." << frame.hands()[0].palmNormal().dot(frame.hands()[1].palmNormal()) << "   " << mainGaucheOuverte << "  "<< mainDroiteOuverte << std::endl;

	}
	cout << "\rselecting = " << (frame.timestamp() - referenceTimeStamp) / 1000000.0 << " ; unselecting = " << (frame.timestamp() - endReferenceTimeStamp) / 1000000.0 << flush;
	//cout << "selecting = " << referenceTimeStamp << " ; unselecting = " << endReferenceTimeStamp << " ; time = " << frame.timestamp() << endl;
#define SEL 500000.0
	bool selection = (frame.timestamp() - referenceTimeStamp) > SEL, deselection = (frame.timestamp() - endReferenceTimeStamp) > SEL;


	switch (currentState) {
	case GestureState::WaitState:
	{
		if (displayMenu && (frame.timestamp() - deselectFrame.timestamp() > 2000000.0)){
			startFrameMenu = frame;

			initHand1 = Hand(hands.leftmost());
			initHand2 = Hand(hands.rightmost());
			currentState = GestureState::SelectMenu;
			std::cout << "MENU SELECTIONNE !" << endl;

		}
		plop2 = true;
		//	cout << "STATE = WAIT" << endl;
		ss_data << "&state=wait";
		ss_data << "&loading=" << (frame.timestamp() - referenceTimeStamp) / SEL;
		if (selection)
		{
			initHand1 = Hand(hands.leftmost());
			initHand2 = Hand(hands.rightmost());
			//std::cout << "MAISON SELECTIONNEE. Centre des deux mains =   " << (hand1.palmPosition() + hand2.palmPosition()) / 2 << std::endl;

			initHand1x = initHand1.palmPosition().x;
			initHand2x = initHand2.palmPosition().x;
			initHand1y = initHand1.palmPosition().y;
			initHand2y = initHand2.palmPosition().y;
			initHand1z = initHand1.palmPosition().z;
			initHand2z = initHand2.palmPosition().z;

			distanceHands = initHand2.palmPosition().distanceTo(initHand1.palmPosition());
			startFrame = frame;
			std::cout << "MAISON SELECTIONNEE" << endl;
			currentState = GestureState::SelectionState;
		}
		break;
	}
	case GestureState::SelectMenu:{
		cout << "select_menu" << endl;

		hand1 = Hand(hands.leftmost());
		hand2 = hands.rightmost();
		ss_data << "&state=selectmenu&loading=" << (frame.timestamp() - endReferenceTimeStamp) / SEL;

	//	cout << "Je suis dans SelectMenu !  " << abs(hand1.fingers()[1].bone(boneTypeD).center().x - initHand1.fingers()[1].bone(boneTypeD).center().x) <<endl;
		if (abs(hand1.fingers()[1].bone(boneTypeD).center().x - initHand1.fingers()[1].bone(boneTypeD).center().x) > 80) {
			std::cout << "MENU DESELECTIONNE !" << endl;
			currentState = GestureState::DeselectionMenu;
	//		menuSelected = true;      
		}
		
	//	if (menuSelected)
	//		currentState = GestureState::WaitState;
		break;
	}


	case GestureState::DeselectionMenu:{
		hand1 = Hand(hands.leftmost());
		hand2 = hands.rightmost();


		const FingerList fingers1 = hand1.fingers();
		const FingerList fingers2 = hand2.fingers();
		nbFingersFront = 0;
		for (FingerList::const_iterator fl = fingers1.begin(); fl != fingers1.end(); ++fl) {
			const Finger finger = *fl;

			for (int b = 0; b < 4; ++b) {
				Bone::Type boneType = static_cast<Bone::Type>(b);
				Bone bone = finger.bone(boneType);

				if (((fingerNames[finger.type()] == "Index") && (boneNames[boneType] == "Proximal") && finger.direction().z < -0.8) ||
					((fingerNames[finger.type()] != "Index") && (fingerNames[finger.type()] != "Thumb") && (boneNames[boneType] == "Proximal") && finger.direction().y < 0))
					nbFingersFront++;
			}
		}
		if (hand1.isLeft()) {
			if (nbFingersFront == 4) {
				if (!pointingLeftHand) {
					leftHandFirstSelect = hand1;
				}
				pointingLeftHand = true;
			}
			else {
				pointingLeftHand = false;
			}
		}
		else if (hand1.isRight()) {
			if (nbFingersFront == 4) {
				if (!pointingRightHand) {
					rightHandFirstSelect = hand1;
				}
				pointingRightHand = true;
			}
			else
				pointingRightHand = false;
		}




		nbFingersFront = 0;

		for (FingerList::const_iterator fl = fingers2.begin(); fl != fingers2.end(); ++fl) {
			const Finger finger = *fl;

			for (int b = 0; b < 4; ++b) {
				Bone::Type boneType = static_cast<Bone::Type>(b);
				Bone bone = finger.bone(boneType);

				if (((fingerNames[finger.type()] == "Index") && (boneNames[boneType] == "Proximal") && finger.direction().z < -0.8) ||
					((fingerNames[finger.type()] != "Index") && (fingerNames[finger.type()] != "Thumb") && (boneNames[boneType] == "Proximal") && finger.direction().y < 0))
					nbFingersFront++;
			}
		}
		if (hand2.isLeft()) {
			if (nbFingersFront == 4) {
				if (!pointingLeftHand)
					leftHandFirstSelect = hand2;
				pointingLeftHand = true;
			}
			else
				pointingLeftHand = false;
		}
		else if (hand2.isRight()) {
			if (nbFingersFront == 4) {
				if (!pointingRightHand)
					rightHandFirstSelect = hand2;
				pointingRightHand = true;
			}
			else
				pointingRightHand = false;
		}



		//	cout << "pointing Left =  " << pointingLeftHand << "   pointing Right   =  " << pointingRightHand << endl;
		//	cout << abs(leftHandFirstSelect.palmPosition().x - hand1.palmPosition().x) << endl;
		if (pointingLeftHand && (abs(leftHandFirstSelect.palmPosition().x - hand1.palmPosition().x)<20) && leftHandFirstSelect.palmPosition().distanceTo(hand1.palmPosition())>30) {
			cout << "Pointing left " << endl;
			ss_data << "&pointing" << true;
		}
		else
			ss_data << "&pointing" << false;
		if (pointingRightHand && (abs(rightHandFirstSelect.palmPosition().x - hand2.palmPosition().x)<20) && rightHandFirstSelect.palmPosition().distanceTo(hand2.palmPosition())>30) {
			cout << "Pointing right " << endl;
			ss_data << "&pointing" << true;
		}
		else
			ss_data << "&pointing" << false;

		//	cout << "Je suis dans SelectMenu !  " << abs(phand1.fingers()[1].bone(boneTypeD).center().x - initHand1.fingers()[1].bone(boneTypeD).center().x) <<endl;

		ss_data << "&state=deselectmenu&loading=0.0";

		//	cout << "Je suis dans SelectMenu !  " << abs(hand1.fingers()[1].bone(boneTypeD).center().x - initHand1.fingers()[1].bone(boneTypeD).center().x) <<endl;
		if (abs(hand1.fingers()[1].bone(boneTypeD).center().x - initHand1.fingers()[1].bone(boneTypeD).center().x) < 10) {
			std::cout << "MENU DESELECTIONNE !" << endl;
			//	menuSelected = true;
			displayMenu = false;
			deselectFrame = frame;
			currentState = GestureState::DeselectMenu;
		}


		break;
	}



	case GestureState::DeselectMenu:{

		currentState = GestureState::WaitState;
		break;
	}
	case GestureState::SelectionState:
	{
		ss_data << "&state=selected";
		ss_data << "&loading=" << 1.0 - (frame.timestamp() - endReferenceTimeStamp) / SEL;

		//cout << "STATE = SELECTION" << endl;
		distanceHands = initHand2.palmPosition().distanceTo(initHand1.palmPosition());


		
		// std::cout << "MAISON DEJA SELECTIONNEE. Centre des deux mains =   " << (hand1.palmPosition() + hand2.palmPosition()) / 2 << std::endl;
		
	//	if (frame.hands()[0].palmNormal().dot(frame.hands()[1].palmNormal()) < -0.8 && plop2) {
	//		secondFrame = frame; plop2 = false;
	//	}
		rotationOfHand1x = hand1.rotationAngle(startFrame, Vector(1, 0, 0));
		rotationOfHand2x = hand2.rotationAngle(startFrame, Vector(1, 0, 0));
		rotationOfHand1y = hand1.rotationAngle(startFrame, Vector(0, 1, 0));
		rotationOfHand2y = hand2.rotationAngle(startFrame, Vector(0, 1, 0));
		rotationOfHand1z = hand1.rotationAngle(startFrame, Vector(0, 0, 1));
		rotationOfHand2z = hand2.rotationAngle(startFrame, Vector(0, 0, 1));

	//	cout << "COORDONNNESS       " << abs(rotationOfHand1y * 180 / M_PI) << "    " << rotationOfHand1y * 180 / M_PI - rotationOfHand2y * 180 / M_PI << "    " << abs(hand1.palmPosition().z - hand2.palmPosition().z) << endl;
		//cout << "ROTATION Y !!                                " << abs(rotationOfHand1z * 180 / M_PI) << "    " << abs(rotationOfHand1z * 180 / M_PI) - abs(rotationOfHand2z * 180 / M_PI) << endl;
		// Pour l'instant, on utilise comme r�f�rence la premi�re frame d�s selection de la maison (startframe)
		// R�fl�chir s'il faut utiliser comme r�f�rence la frame pr�c�dente, et faire += pour avoir la valeur totale (rotation comme translation)

		//cout << "ROATION  EN Z    ----------------- " << abs(rotationOfHand1z * 180 / M_PI) << endl;

		if (deselection){ //unselect
			nbFrameDeselect = 0;
			currentState = GestureState::EndTransformationState;
		}
	/*	else if (((abs(initHand1x - hand1.palmPosition().x) > 50) && (abs(initHand2x - hand2.palmPosition().x) > 50))
			|| ((abs(initHand1y - hand1.palmPosition().y) > 50) && (abs(initHand2y - hand2.palmPosition().y) > 50))
			|| ((abs(initHand1z - hand1.palmPosition().z) > 50) && (abs(initHand2z - hand2.palmPosition().z) > 50))) {
			currentState = GestureState::TranslationState;

		}*/

		else if ((abs(rotationOfHand1y * 180 / M_PI) > 20)
			&& (rotationOfHand1y * 180 / M_PI - rotationOfHand2y * 180 / M_PI < 20) && (abs(hand1.palmPosition().z - hand2.palmPosition().z) >10)) {

			std::cout << "rotation en Y !   " << rotationOfHand1y * 180 / M_PI << "  " << "rotation 2  " << rotationOfHand2y * 180 / M_PI << endl;
			currentState = GestureState::RotationState;
			rotationAxis = Vector(0, 1, 0);
		}

		else if ((abs(rotationOfHand1z * 180 / M_PI)>30) && (abs(hand1.palmPosition().y - hand2.palmPosition().y) >10) 
			&& !(abs(hand1.palmPosition().y - initHand1.palmPosition().y)<10) && !(abs(hand2.palmPosition().y - initHand2.palmPosition().y)<10)) {
			
			std::cout << "rotation en Z !   " << rotationOfHand1z * 180 / M_PI << "  " << "rotation 2  " << rotationOfHand2z * 180 / M_PI << endl;
			currentState = GestureState::RotationState;
			rotationAxis = Vector(0, 0, 1);
		}
		else if ((abs(rotationOfHand1x * 180 / M_PI)>40) && (rotationOfHand1x * 180 / M_PI - rotationOfHand2x * 180 / M_PI < 20)) {
			std::cout << "rotation en X !   " << rotationOfHand1x * 180 / M_PI << "  " << "rotation 2  " << rotationOfHand2x * 180 / M_PI << endl;
			currentState = GestureState::RotationState;
			rotationAxis = Vector(1, 0, 0);
		}

		//	std::cout << " Distance  " << distanceHands << std::endl;
		//cout << "newDistance = " << newDistanceHands << endl;
		//cout << "Diff�rence  = " << abs(distanceHands - newDistanceHands) << endl;

		else {

			//cout << "Pourquoiiiiiiiiiiiiiiiiiiiiiii   " << abs(rotationOfHand1y * 180 / M_PI) << endl;
			previousFrame = frame;
			//	std::cout << "Main 1 =   " << hand1.palmPosition() << std::endl;
			//	std::cout << "Main 2 =   " << hand2.palmPosition() << std::endl;

			newDistanceHands = hand2.palmPosition().distanceTo(hand1.palmPosition());
		//	cout << " DIFFF XX " << abs(hand1.palmPosition().x - initHand1.palmPosition().x) << endl;
			//Si les mains restent � une distance � peu pr�s stable
			if (abs(distanceHands - newDistanceHands) > 50 && (hand1.palmPosition().y - hand2.palmPosition().y < 30 )) { // && Pas rotation !
				currentState = GestureState::ZoomState;
			}

#define TRANS 20
			else if ((((abs(initHand1x - hand1.palmPosition().x) >TRANS) && (abs(initHand2x - hand2.palmPosition().x) > TRANS) && ((initHand1x - hand1.palmPosition().x)*(initHand2x - hand2.palmPosition().x)>0))
				|| ((abs(initHand1y - hand1.palmPosition().y) > TRANS) && (abs(initHand2y - hand2.palmPosition().y) > TRANS) && ((initHand1y - hand1.palmPosition().y)*(initHand2y - hand2.palmPosition().y)>0))
				|| ((abs(initHand1z - hand1.palmPosition().z) > TRANS) && (abs(initHand2z - hand2.palmPosition().z) > TRANS) && (abs(hand1.palmPosition().x - initHand1.palmPosition().x)<20)
				&& (abs(hand1.palmPosition().z - hand2.palmPosition().z)<TRANS) && (abs(hand2.palmPosition().x - initHand2.palmPosition().x)<TRANS)))
				&& (abs(hand1.palmPosition().y - hand2.palmPosition().y)<TRANS) && (abs(initHand1.palmPosition().y - initHand2.palmPosition().y)<TRANS)
				&& ((initHand1z - hand1.palmPosition().z)*(initHand2z - hand2.palmPosition().z)>0))  {

			//	cout << abs(distanceHands - newDistanceHands) << endl;

			//	cout << "HAHAAHAHAHAHAHAAH" << endl;
				currentState = GestureState::TranslationState;

			}
			

		}
		break;
	}
	case GestureState::EndTransformationState:
	{
		std::cout << "STATE = STOP" << endl;
		std::cout << "MAISON DESELECTIONNEE." << std::endl;
		ss_data << "&state=unselect";
		ss_data << "&loading=" << 0.0;
		nbFrameDeselect++;
		if (nbFrameDeselect > 4) {
			currentState = GestureState::WaitState;
		}
		break;
	}
	case GestureState::ZoomState:
	{
		cout << "STATE = ZOOM : " << abs((hand1.palmPosition().x - hand2.palmPosition().x) / (initHand1x - initHand2x));
		ss_data << "&state=zoom";
		ss_data << "&loading=" << 1.0 - (frame.timestamp() - endReferenceTimeStamp) / SEL;
		ss_data << "&x=1&y=1&z=1&param=" << abs((hand1.palmPosition().x - hand2.palmPosition().x) / (initHand1x - initHand2x));


		if (deselection){ //unselect
			nbFrameDeselect = 0;
			currentState = GestureState::EndTransformationState;
		}

		break;
	}
	case GestureState::TranslationState:
	{
		ss_data << "&loading=" << 1 - (frame.timestamp() - endReferenceTimeStamp) / SEL;

		ss_data << "&state=translate";
			//cout << "HAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << abs(hand1.palmPosition().x - initHand1.palmPosition().x) << "  -  " <<abs(hand2.palmPosition().x - initHand2.palmPosition().x) << endl;
		//cout << " Y    = " << abs(hand1.palmPosition().x - hand2.palmPosition().x) << endl;
		if ((abs(initHand1x - hand1.palmPosition().x) > 30) && (abs(initHand2x - hand2.palmPosition().x) > 30)) {
			std::cout << "STATE = TRANSLATION EN X" << endl;
			ss_data << "&x=1&y=0&z=0&param=" << initHand1x - hand1.palmPosition().x;
		}
		else if ((abs(initHand1y - hand1.palmPosition().y) > 30) && (abs(initHand2y - hand2.palmPosition().y) > 30)) {
			std::cout << "STATE = TRANSLATION EN Y" << endl;
			// cout << "DAMN IT Y !!  " << abs(hand1.palmPosition().y - hand2.palmPosition().y) << endl;
			ss_data << "&x=0&y=1&z=0&param=" << initHand1y - hand1.palmPosition().y;
		}
		else if ((abs(initHand1z - hand1.palmPosition().z) > 30) && (abs(initHand2z - hand2.palmPosition().z) > 30) && (abs(hand1.palmPosition().x - initHand1.palmPosition().x) < 20) && (abs(hand2.palmPosition().x - initHand2.palmPosition().x)<20))
		{
			std::cout << "STATE = TRANSLATION EN Z" << endl;
			//cout << " DAMN IT ! " << abs(hand1.palmPosition().y - hand2.palmPosition().y) << endl;
			ss_data << "&x=0&y=0&z=1&param=" << initHand1z - hand1.palmPosition().z;
		}
		
		if (deselection){ //unselect
			nbFrameDeselect = 0;
			currentState = GestureState::EndTransformationState;
		}
		break;
	}
	case GestureState::RotationState:
	{
		ss_data << "&state=rotate";
		ss_data << "&loading=" << 1 - (frame.timestamp() - endReferenceTimeStamp) / SEL;
		//cout << abs(rotationOfHand1z * 180 / M_PI) << "      -     " << abs(rotationOfHand2z * 180 / M_PI) << endl;
		//cout << hand1.palmPosition().distanceTo(initHand1.palmPosition()) << endl;
		std::cout << "STATE = ROTATION" << endl;
		

		if (deselection){ //unselect
			nbFrameDeselect = 0;
			currentState = GestureState::EndTransformationState;
		}
		float rotationOfHand1, rotationOfHand2;
		rotationOfHand1 = hand1.rotationAngle(startFrame, rotationAxis);
		rotationOfHand2 = hand2.rotationAngle(startFrame, rotationAxis);
		std::cout << "Rotation : A" << rotationAxis << " G" << rotationOfHand1 * 180 / M_PI << " D" << rotationOfHand2 * 180 / M_PI << endl;
		ss_data << "&x=" << rotationAxis.x << "&y=" << rotationAxis.y << "&z="<<rotationAxis.z << "&param=" << rotationOfHand1 * 180 / M_PI;
		break;
	}
	default:
	{
		std::cout << "Cas non trait� actuellement." << endl;
	}
	}



	closedLeftFist = false;
	closedRightFist = false;
	mainGaucheOuverte = false;
	mainDroiteOuverte = false;
	// Get tools
	const ToolList tools = frame.tools();
	for (ToolList::const_iterator tl = tools.begin(); tl != tools.end(); ++tl) {
		const Tool tool = *tl;
		/*	std::cout << std::string(2, ' ') << "Tool, id: " << tool.id()
		<< ", position: " << tool.tipPosition()
		<< ", direction: " << tool.direction() << std::endl;*/
	}

	// Get gestures
	const GestureList gestures = frame.gestures();
	for (int g = 0; g < gestures.count(); ++g) {
		Gesture gesture = gestures[g];

		switch (gesture.type()) {
		case Gesture::TYPE_CIRCLE:
		{
									 CircleGesture circle = gesture;
									 std::string clockwiseness;

									 if (circle.pointable().direction().angleTo(circle.normal()) <= PI / 2) {
										 clockwiseness = "clockwise";
									 }
									 else {
										 clockwiseness = "counterclockwise";
									 }

									 // Calculate angle swept since last frame
									 float sweptAngle = 0;
									 if (circle.state() != Gesture::STATE_START) {
										 CircleGesture previousUpdate = CircleGesture(controller.frame(1).gesture(circle.id()));
										 sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * PI;
									 }
									 /*		 std::cout << std::string(2, ' ')
									 << "Circle id: " << gesture.id()
									 << ", state: " << stateNames[gesture.state()]
									 << ", progress: " << circle.progress()
									 << ", radius: " << circle.radius()
									 << ", angle " << sweptAngle * RAD_TO_DEG
									 << ", " << clockwiseness << std::endl;*/
									 break;
		}
		case Gesture::TYPE_SWIPE:
		{
									SwipeGesture swipe = gesture;
								/*	std::cout << std::string(2, ' ')
									<< "Swipe id: " << gesture.id()
									<< ", state: " << stateNames[gesture.state()]
									<< ", direction: " << swipe.direction()
									<< ", speed: " << swipe.speed() << std::endl;*/
									break;
		}
		case Gesture::TYPE_KEY_TAP:
		{
									  KeyTapGesture tap = gesture;
									  /*std::cout << std::string(2, ' ')
										  << "Key Tap id: " << gesture.id()
										  << ", state: " << stateNames[gesture.state()]
										  << ", position: " << tap.position()
										  << ", direction: " << tap.direction() << std::endl;*/
									  break;
		}
		case Gesture::TYPE_SCREEN_TAP:
		{
										 ScreenTapGesture screentap = gesture;
										/* std::cout << std::string(2, ' ')
											 << "Screen Tap id: " << gesture.id()
											 << ", state: " << stateNames[gesture.state()]
											 << ", position: " << screentap.position()
											 << ", direction: " << screentap.direction() << std::endl;*/
										 break;
		}
		default:
			std::cout << std::string(2, ' ') << "Unknown gesture type." << std::endl;
			break;
		}
	}

	if (!frame.hands().isEmpty() || !gestures.isEmpty()) {
		/*std::cout << std::endl;*/
	}

	Frame prevFrame = frame;
	
	if (sendingNeeded) {
		string data;
		ss_data >> data;
		//sender.sendData(data);
	}
	sendingNeeded = !sendingNeeded;
}

void SampleListener::onFocusGained(const Controller& controller) {
	std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Controller& controller) {
	std::cout << "Focus Lost" << std::endl;
}

void SampleListener::onDeviceChange(const Controller& controller) {
	std::cout << "Device Changed" << std::endl;
	const DeviceList devices = controller.devices();

	for (int i = 0; i < devices.count(); ++i) {
		std::cout << "id: " << devices[i].toString() << std::endl;
		std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
	}
}

void SampleListener::onServiceConnect(const Controller& controller) {
	std::cout << "Service Connected" << std::endl;
}

void SampleListener::onServiceDisconnect(const Controller& controller) {
	std::cout << "Service Disconnected" << std::endl;
}

int main(int argc, char** argv) {
	std::cout << "Entr�e dans le main" << std::endl;
	// Create a sample listener and controller
	SampleListener listener;
	Controller controller;
	std::cout << "Classes instanci�es" << std::endl;
	// Have the sample listener receive events from the controller
	controller.addListener(listener);

	if (argc > 1 && strcmp(argv[1], "--bg") == 0)
		controller.setPolicyFlags(Leap::Controller::POLICY_BACKGROUND_FRAMES);

	// Keep this process running until Enter is pressed
	std::cout << "Press Escape to quit..." << std::endl;
	char key_pressed = 0;
	while (key_pressed != escape) {
		key_pressed = _getch();
		std::cout << (int)key_pressed << endl;
	}

	// Remove the sample listener when done
	controller.removeListener(listener);

	return 0;
}
//
//int _tmain(int argc, _TCHAR* argv[])
//{
//	main(argc, (char*[]) argv)
//	return 0;
//}