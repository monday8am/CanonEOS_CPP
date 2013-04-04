#pragma once

class ActionEvent {
private:
	std::string _command;
	void* _arg;
public:
	const std::string getActionCommand() const{ return _command; }
	void* getArg() const{ return _arg; }

	ActionEvent(std::string command,void* arg=0) : 
	 _command(command), _arg(arg) {}

	virtual ~ActionEvent() {}
};
