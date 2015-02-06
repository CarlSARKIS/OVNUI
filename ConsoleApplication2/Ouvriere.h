#pragma once

#include <string>
class Ouvriere
{
public:
	Ouvriere();
	void initConnection(std::wstring ip);
	void sendData(std::string form);
	~Ouvriere();
};

