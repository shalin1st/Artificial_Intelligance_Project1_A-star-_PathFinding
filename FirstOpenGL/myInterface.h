#pragma once
#include"myPhysics.h"
class myInterface : public myPhysics
{
public: 
	
	virtual void ReadFileToToken(std::ifstream &file, std::string token) = 0;
	virtual std::vector<object1> InitializePhysics(std::string file) = 0;
	virtual void PhysicsStep(double deltaTime, force &wforce, force &sforce, force &leftforce, force &rightforce, force &upforce, std::vector<glm::vec3> &disp, std::vector<glm::vec3> &rot, std::vector<object1> &my, std::vector<int> &collision) = 0;

};