#ifndef SINGLETON_H
#define SINGLETON_H

#include <utility>
#include <windows.h>
#include <stdint.h>

#include <rpc.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <array>
#include <tchar.h> 
#include <stdio.h>
#include <tlhelp32.h>
#include <timeapi.h>

#include <fstream>
#include <iterator>
#include <algorithm>

#include <functional>
#include <map>
#include <vector>
#include <tuple>
#include <array>
#include <excpt.h>

#include "TMFieldScene.h"
#include "SControlContainer.h"
#include "SGrid.h"
#include "Mission.h"
#include "MrItemMix.h"
#include "TMGround.h"
#include "TMHuman.h"

// Generic, lazy, singleton base-class
// that supports any number of arguments
template<typename T>
class Singleton
{
public:
	// Gets the only instance of the inherited class
	template<typename ...Args>
	static T& instance(Args... args)
	{
		static T instance(std::forward<Args>(args)...);

		return instance;
	}

protected:
	Singleton() {};
	virtual ~Singleton() {}
};

#endif