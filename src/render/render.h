/*
 * Copyright 2012 Moritz Hilscher
 *
 * This file is part of mapcrafter.
 *
 * mapcrafter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mapcrafter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mapcrafter.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RENDER_H_
#define RENDER_H_

#include "mc/cache.h"

#include "render/image.h"
#include "render/textures.h"
#include "render/tile.h"

#include <map>
#include <boost/filesystem.hpp>

#define BLOCK_NORTH mc::BlockPos(0, -1, 0)
#define BLOCK_SOUTH mc::BlockPos(0, 1, 0)
#define BLOCK_EAST mc::BlockPos(1, 0, 0)
#define BLOCK_WEST mc::BlockPos(-1, 0, 0)

namespace fs = boost::filesystem;

namespace mapcrafter {
namespace render {

class TileTopBlockIterator {
private:
	int block_size, tile_size;

	bool is_end;
	int min_row, max_row;
	int min_col, max_col;
	mc::BlockPos top;
public:
	TileTopBlockIterator(const TilePos& tile, int block_size, int tile_size);
	~TileTopBlockIterator();

	void next();
	bool end() const;

	mc::BlockPos current;
	int draw_x, draw_y;
};

class BlockRowIterator {
public:
	BlockRowIterator(const mc::BlockPos& block);
	~BlockRowIterator();

	void next();
	bool end() const;

	mc::BlockPos current;
};

struct RenderBlock {

	int x, y;
	bool transparent;
	Image image;
	mc::BlockPos pos;
	uint8_t id, data;

	bool operator<(const RenderBlock& other) const;
};

class TileRenderer {
private:
	mc::WorldCache& world;
	const BlockTextures& textures;
public:
	TileRenderer(mc::WorldCache& world, const BlockTextures& textures);
	~TileRenderer();

	uint16_t checkNeighbors(const mc::BlockPos& pos, uint16_t id, uint16_t data,
	        const mc::Chunk* chunk);
	void renderTile(const TilePos& pos, Image& tile);
};

}
}

#endif /* RENDER_H_ */