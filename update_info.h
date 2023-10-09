#ifndef __UPDATEINFO_H__
#define __UPDATEINFO_H__
#include <vector>
#include "House.h"
#include "Tree.h"
#include "Snowball.h"
#include "Lamppost.h"
#include "Statue.h"


struct keyboard_key_tracking {
	bool w_key;
	bool a_key;
	bool s_key;
	bool d_key;
	bool o_key;
};
struct update_info {
	short accelerating;	
	short h_turning;	
	short v_turning;

	std::vector<House> *houses;
	std::vector<Tree> *trees;
	std::vector<SnowBall>* snowballs;
	std::vector<Lamppost>* lampposts;
	Statue* statue;
};

#endif