#ifndef EO_GFX_POOL_HPP
#define EO_GFX_POOL_HPP

#include "engine.hpp"

#include <vector>

class GFX_Pool
{
	private:
		std::vector<Engine::Graphic> m_atlas;
};

class GFX_Ref; // TODO

#endif // EO_GFX_POOL_HPP
