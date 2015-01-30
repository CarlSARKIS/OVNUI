// ConsoleApplication2.cpp : définit le point d'entrée pour l'application console.
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

void SampleListener::onInit(const Controller& controller) {
	std::cout << "Initialized" << std::endl;
	wstring ip;
	cout << "Please enter the phone IP adress :" << endl;
	wcin >> ip;
	wcout << "Connecting to " << ip << "..." << endl;
	sender.initConnection(ip);
	wcout << "Successfully connected to " << ip << " !" << endl;
	sender.sendData("Message=Connected to the computer !");
	sendingNeeded = true;
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
int nbFrameCatch = 0;
int nbFrameUncatch = 0;
float rotationOfHand1x, rotationOfHand1y, rotationOfHand1z, rotationOfHand2x, rotationOfHand2y, rotationOfHand2z;
Hand initHand1;
Hand initHand2;
Hand hand1;
Hand hand2;
Frame previousFrame;
Frame startFrame;
float initHand1x, initHand2x, initHand1y, initHand2y, initHand1z, initHand2z;
void SampleListener::onFrame(const Controller& controller) {
	// Get the most recent frame and report some basic information
	
	std::stringstream ss_data; // data send to the phone


	const Frame frame = controller.frame();
	//rotationOfHand1 = hand1.rotationAngle(previousFrame);
	//previousFrame = frame;
	controller.config().setFloat("Gesture.Swipe.MinVelocity", 100);
	ss_data << "frame=" << frame.id();
	ss_data << "&time=" << (double)(frame.timestamp())/1000000.0;
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
	
	HandList hands = frame.hands();
	//std::cout << frame.hands().count() << std::endl;
	for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
		
		int comp = 0;
		//	closedLeftFist = false;
		//	closedRightFist = false;
		// Get the first hand
		const Hand hand = *hl;
		std::string handType = hand.isLeft() ? "Left_hand" : "Right_hand";
		std::cout << std::string(2, ' ') << handType << ", id: " << hand.id()
		<< ", palm position: " << hand.palmPosition() << std::endl;
		ss_data << "&" << handType << "=" << hand.palmPosition().x << "/" << hand.palmPosition().y << "/" << hand.palmPosition().z;

		
		// Get the hand's normal vector and direction
		const Vector normal = hand.palmNormal();
		const Vector direction = hand.direction();
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
		int nbFingersFront = 0;
		float proximalYMin = 1000;
		float proximalYMax = 0;
		for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
			const Finger finger = *fl;
			/*	std::cout << std::string(4, ' ') <<  fingerNames[finger.type()]
			<< " finger, id: " << finger.id()
			<< ", length: " << finger.length()
			<< "mm, width: " << finger.width() << std::endl;*/

			// Get finger bones

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
				if (((fingerNames[finger.type()] != "Thumb") && (boneNames[boneType] == "Proximal") && (bone.direction().z >= 0.9)))
					nbFingersFront++;
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
		
	//	std::cout << handType << "   " << normal.y << "   " << hand.palmPosition().y << "  " << nbFingersFront << std::endl;
		
		if ((handType == "Left hand") && (normal.x > 0.8 && hand.palmPosition().x < -70) && (nbFingersFront == 4)) {
		//	std::cout << "Main gauche ouverte." << std::endl;
			mainGaucheOuverte = true;
		}
		else if ((handType == "Right hand") && (normal.x < -0.8 && hand.palmPosition().x >70) && (nbFingersFront == 4)){
			//std::cout << "Main droite ouverte." << std::endl;
			mainDroiteOuverte = true;
		}		
		if ((handType == "Left hand") && (normal.y <-0.97)) {
			//	std::cout << "Main gauche bas." << std::endl;
			mainGaucheBas = true;
		}
		else if ((handType == "Right hand") && (normal.y <-0.97)){
			//std::cout << "Main droite ouverte." << std::endl;
			mainDroiteBas = true;
		}
		nbFingersFront = 0;
		proximalYMin = 1000;
		proximalYMax = 0;
		//	else if ((handType == "Right hand") && (thumbPositionX > ringPositionX) && ()

		if (comp == 4) {
			//std::cout << "La " << handType << " est un poing fermé." << std::endl;
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
		//		std::cout << "Deux poings fermés." << std::endl;


	}
	if (frame.hands().count() == 2) {
		//std::cout << "HEYEHEYEHEYEHEY" << std::endl;
		if (mainDroiteOuverte && mainGaucheOuverte) {
			//std::cout << "Translation Powered  " << nbFrameCatch << std::endl;
			nbFrameCatch++;
		}

		if (nbFrameCatch == 120) {
			
			initHand1 = Hand(hands.leftmost());
			initHand2 = Hand(hands.rightmost());
			std::cout << "MAISON SELECTIONNEE. Centre des deux mains =   " << (hand1.palmPosition() + hand2.palmPosition()) / 2 << std::endl;

			initHand1x = initHand1.palmPosition().x;
			initHand2x = initHand2.palmPosition().x;
			 initHand1y = initHand1.palmPosition().y;
			 initHand2y = initHand2.palmPosition().y;
			 initHand1z = initHand1.palmPosition().z;
			 initHand2z = initHand2.palmPosition().z;
			 distanceHands = sqrt(pow(initHand2.palmPosition().x - initHand1.palmPosition().x, 2) + pow(initHand2.palmPosition().y - initHand1.palmPosition().y, 2) + pow(initHand2.palmPosition().z - initHand1.palmPosition().z, 2));
			// cout << "Init Distance  = " << distanceHands << endl;
			 startFrame = frame;
			 
		}
		if (nbFrameCatch>120) {
		//	newFrame = frame;
			distanceHands = sqrt(pow(initHand2.palmPosition().x - initHand1.palmPosition().x, 2) + pow(initHand2.palmPosition().y - initHand1.palmPosition().y, 2) + pow(initHand2.palmPosition().z - initHand1.palmPosition().z, 2));
			
			hand1 = Hand(hands.leftmost());
			hand2 = hands.rightmost();
			std::cout << "MAISON SELECTIONNEE. Centre des deux mains =   " << (hand1.palmPosition() + hand2.palmPosition()) / 2 << std::endl;
			rotationOfHand1x = hand1.rotationAngle(startFrame, Vector(1, 0, 0));
			rotationOfHand2x = hand2.rotationAngle(startFrame, Vector(1, 0, 0));
			rotationOfHand1y = hand1.rotationAngle(startFrame, Vector(0, 1, 0));
			rotationOfHand2y = hand2.rotationAngle(startFrame, Vector(0, 1, 0));
			rotationOfHand1z = hand1.rotationAngle(startFrame, Vector(0, 0, 1));
			rotationOfHand2z = hand2.rotationAngle(startFrame, Vector(0, 0, 1));



			// Pour l'instant, on utilise comme référence la première frame dès selection de la maison.
			// Réfléchir s'il faut utiliser comme référence la frame précédente, et faire += pour avoir la valeur totale (rotation comme translation)



			cout << "rotation 1   " << rotationOfHand1x*180/M_PI << "  " << "rotation 2  " << rotationOfHand2x*180/M_PI << endl;
			previousFrame = frame;
			//	std::cout << "Main 1 =   " << hand1.palmPosition() << std::endl;
			//	std::cout << "Main 2 =   " << hand2.palmPosition() << std::endl;
			
			newDistanceHands = sqrt(pow(hand2.palmPosition().x - hand1.palmPosition().x, 2) + pow(hand2.palmPosition().y - hand1.palmPosition().y, 2) + pow(hand2.palmPosition().z - hand1.palmPosition().z, 2));

		//	std::cout << " Distance  " << distanceHands << std::endl;
			//cout << "newDistance = " << newDistanceHands << endl;
			//cout << "Différence  = " << abs(distanceHands - newDistanceHands) << endl;
			if (abs(distanceHands - newDistanceHands) < 80) { // && Pas rotation !
			//	std::cout << "Translation" << std::endl;
			//	cout << "Calcul x1 " << initHand1x - hand1.palmPosition().x << "Calcul x2 " << initHand2x - hand2.palmPosition().x << endl;
				if ((initHand1x - hand1.palmPosition().x >50) && (initHand2x - hand2.palmPosition().x >50)) {
					std::cout << "Translation en x" << std::endl;
				}
				else if ((initHand1y - hand1.palmPosition().y >50) && (initHand2y - hand2.palmPosition().y >50)) {
					std::cout << "Translation en y" << std::endl;
				}
				else if ((initHand1z - hand1.palmPosition().z >50) && (initHand2z - hand2.palmPosition().z >50)) {
					std::cout << "Translation en z" << std::endl;
				}
			}
		/*	else if ((abs(distanceHands - newDistanceHands) < 80) && ) {

			}*/
			else {
				std::cout << "Zoom" << std::endl;
			}
			
			


			//houseCatched = true;
			if (mainGaucheBas && mainDroiteBas){
			//	std::cout << "nbFrame uncatched ++." << std::endl;
				nbFrameUncatch++;
			//	cout << nbFrameUncatch << endl;
			}

			if (nbFrameUncatch > 120){
				std::cout << "MAISON DESELECTIONNEE." << std::endl;
				nbFrameCatch = 0;
				nbFrameUncatch = 0;
			}
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
			/*std::cout << std::string(2, ' ')
			<< "Swipe id: " << gesture.id()
			<< ", state: " << stateNames[gesture.state()]
			<< ", direction: " << swipe.direction()
			<< ", speed: " << swipe.speed() << std::endl;*/
			break;
		}
		case Gesture::TYPE_KEY_TAP:
		{
			KeyTapGesture tap = gesture;
			std::cout << std::string(2, ' ')
				<< "Key Tap id: " << gesture.id()
				<< ", state: " << stateNames[gesture.state()]
				<< ", position: " << tap.position()
				<< ", direction: " << tap.direction() << std::endl;
			break;
		}
		case Gesture::TYPE_SCREEN_TAP:
		{
			ScreenTapGesture screentap = gesture;
			std::cout << std::string(2, ' ')
				<< "Screen Tap id: " << gesture.id()
				<< ", state: " << stateNames[gesture.state()]
				<< ", position: " << screentap.position()
				<< ", direction: " << screentap.direction() << std::endl;
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
		sender.sendData(data);
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
	std::cout << "Entrée dans le main" << std::endl;
	// Create a sample listener and controller
	SampleListener listener;
	Controller controller;
	std::cout << "Classes instanciées" << std::endl;
	// Have the sample listener receive events from the controller
	controller.addListener(listener);

	if (argc > 1 && strcmp(argv[1], "--bg") == 0)
		controller.setPolicyFlags(Leap::Controller::POLICY_BACKGROUND_FRAMES);

	// Keep this process running until Enter is pressed
	std::cout << "Press Escape to quit..." << std::endl;
	char key_pressed = 0;
	while (key_pressed != escape) {
		key_pressed = _getch();
		cout << (int)key_pressed << endl;
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