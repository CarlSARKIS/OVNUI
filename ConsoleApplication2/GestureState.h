#pragma once

#include <string>
#include "Leap.h"

class GestureState
{
public:
	GestureState();
	~GestureState();
	virtual void onFrame(Leap::Frame &frame);
	virtual std::string data();
	virtual GestureState * nextState();
};


class WaitState : public GestureState
{
public:
	WaitState();
	~WaitState();
};


class SelectionState : public GestureState
{
	Leap::Frame beginFrame;
public:
	SelectionState();
	~SelectionState();
};

class TransformationState : public GestureState
{
	Leap::Frame beginFrame;
public:
	TransformationState();
	~TransformationState();
};

class RotationState : public TransformationState
{
	Leap::Vector axis;
public:
	RotationState();
	~RotationState();
};



class TranslationState : public TransformationState
{
public:
	TranslationState();
	~TranslationState();
};

class Zoom : public TransformationState
{
public:
	Zoom();
	~Zoom();
};

class EndTransformationState : public TransformationState
{
public:
	EndTransformationState();
	~EndTransformationState();
};
