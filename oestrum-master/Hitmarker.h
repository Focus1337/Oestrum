#pragma once
#include "Singleton.hpp"

class CHitmarker : public Singleton<CHitmarker>
{
public:
	float end_time;
	void draw();
	void play_sound();
	void update_end_time();
};