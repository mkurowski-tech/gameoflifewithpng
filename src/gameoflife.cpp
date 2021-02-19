#include <iostream>
#include <string>
#include <iomanip>
#include <memory>
#include <vector>
#include <cassert>
#include <png.h>

using namespace std;

/*
Any live cell with fewer than two live neighbours dies, as if by underpopulation.
Any live cell with two or three live neighbours lives on to the next generation.
Any live cell with more than three live neighbours dies, as if by overpopulation.
Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
*/

using png_byte_table = unique_ptr<png_byte[]>;
struct Pngfile {
	int width, height;
	png_byte color_type;
	png_byte bit_depth;

	png_structp png_ptr;
	png_infop info_ptr;
	int number_of_passes;

	unique_ptr<png_byte_table[]> row_pointers;
	vector<png_bytep> row_ptrs;
};

void gameoflife(vector<bool> &cells, int width, int step);
void writepngfile(const string &file_name, Pngfile &pngfile);
void readpngfile(const string &file_name, Pngfile &pngfile);

int main(int argc, char** args)
try
{
	vector<bool> cells {};
	Pngfile pngfile;

	readpngfile("gamein.png", pngfile);
	cells.resize(pngfile.width * pngfile.height, false);

	for (int y=0; y<pngfile.height; y++) {
		png_byte* row = pngfile.row_pointers[y].get();
		for (int x=0; x<pngfile.width; x++) {
				png_byte* ptr = &(row[x*4]);

				if(ptr[0]!=0 && ptr[1]!=0 && ptr[2]!=0)
					cells[x + pngfile.width*y] = true;
		}
    }

    for(int s=0; s<100; ++s) {
    	cout << s << endl;

		for (int y=0; y<pngfile.height; y++) {
			png_byte* row = pngfile.row_pointers[y].get();
			for (int x=0; x<pngfile.width; x++) {
					png_byte* ptr = &(row[x*4]);

					if(cells[x + pngfile.width*y]) {
						ptr[0]=255;
						ptr[1]=255;
						ptr[2]=255;
						ptr[3]=255;
					} else {
						ptr[0] = 0;
						ptr[1] = 0;
						ptr[2] = 0;
						ptr[3]=255;
					}
			}
		}
		stringstream ss;
		ss << "gameout_" << std::setw(3) << std::setfill('0') << s << ".png";
	    writepngfile(ss.str(), pngfile);

	    // after writing the file, due to rewrite init schema as first picture
		gameoflife(cells, pngfile.width, 1);
    }

} catch (exception &e) {
	cerr << "CAUGHT exception: " << e.what() << endl;
} catch (...) {
	cerr << "CAUGHT ... " << endl;
}

void gameoflife(vector<bool> &board, int boardwidth, int step) {
	assert(step>0);
	assert(board.size() % boardwidth == 0);
	const int boardheight = board.size() / boardwidth;

	auto countcells = [boardwidth, boardheight](const vector<bool> &c, int x, int y, int &alive, int &dead) {
		if (x < 0 || x >= boardwidth) dead++;
		else if (y < 0 || y >= boardheight) dead++;
		else if (c[x + y * boardwidth]>0) alive++;
		else dead++;
	};

	auto countneighbours = [countcells](const vector<bool> &c, int x, int y, int &alive, int &dead) {

		for(int i=-1; i<2; ++i) {
			countcells(c, x+i, y-1, alive, dead);
			countcells(c, x+i, y+1, alive, dead);
		}

		countcells(c, x-1, y, alive, dead);
		countcells(c, x+1, y, alive, dead);
	};

	vector<bool> next;
	next.resize(board.size());
	for(int y = 0; y < boardheight; ++y) {
		for(int x = 0; x < boardwidth; ++x) {
			int alive {}, dead {};
			countneighbours(board, x, y, alive, dead);

			const int idx {x + y * boardwidth};
			if (board[idx] && alive < 2) next[idx] = false;
			if (board[idx] && (alive == 2 || alive == 3)) next[idx] = true;
			if (board[idx] && alive > 3) next[idx] = false;
			if (!board[idx] && alive == 3) next[idx] = true;
		}
	}

	copy(next.begin(), next.end(), board.begin());
}

void readpngfile(const string &file_name, Pngfile &pngfile)
{
		png_byte header[8];

		// open
        FILE *fp = fopen(file_name.c_str(), "rb");
        if (!fp)
        	throw logic_error ("Could not open a file for reading");

        fread(header, 1, 8, fp);
        if (png_sig_cmp(header, 0, 8))
                throw logic_error("File is not a PNG file");

        // init
        pngfile.png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        if (!pngfile.png_ptr)
               throw logic_error("png_create_read_struct failed");

        pngfile.info_ptr = png_create_info_struct(pngfile.png_ptr);
        if (!pngfile.info_ptr)
                throw logic_error("png_create_info_struct failed");

        if (setjmp(png_jmpbuf(pngfile.png_ptr)))
                throw logic_error("Error during init_io");

        png_init_io(pngfile.png_ptr, fp);
        png_set_sig_bytes(pngfile.png_ptr, 8);

        png_read_info(pngfile.png_ptr, pngfile.info_ptr);

        pngfile.width = png_get_image_width(pngfile.png_ptr, pngfile.info_ptr);
        pngfile.height = png_get_image_height(pngfile.png_ptr, pngfile.info_ptr);
        pngfile.color_type = png_get_color_type(pngfile.png_ptr, pngfile.info_ptr);
        pngfile.bit_depth = png_get_bit_depth(pngfile.png_ptr, pngfile.info_ptr);

        pngfile.number_of_passes = png_set_interlace_handling(pngfile.png_ptr);
        png_read_update_info(pngfile.png_ptr, pngfile.info_ptr);

        // read
        if (setjmp(png_jmpbuf(pngfile.png_ptr)))
        	throw logic_error("Error during read_image");

        pngfile.row_pointers = unique_ptr<png_byte_table[]>(new png_byte_table[pngfile.height]);

        for (int y=0; y<pngfile.height; y++)
        	pngfile.row_pointers[y] = unique_ptr<png_byte[]>(new png_byte[png_get_rowbytes(pngfile.png_ptr,pngfile.info_ptr)]);

        pngfile.row_ptrs.resize(pngfile.height);
        for (int y=0; y<pngfile.height; y++)
        	pngfile.row_ptrs[y] = pngfile.row_pointers[y].get();

        png_read_image(pngfile.png_ptr, pngfile.row_ptrs.data());

        fclose(fp);
}

void writepngfile(const string &file_name, Pngfile &pngfile)
{
        FILE *fp = fopen(file_name.c_str(), "wb");
        if (!fp)
        	throw logic_error("Could not open file for writing");

        // init
        pngfile.png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        if (!pngfile.png_ptr)
        	throw logic_error("png_create_write_struct failed");

        pngfile.info_ptr = png_create_info_struct(pngfile.png_ptr);
        if (!pngfile.info_ptr)
        	throw logic_error("png_create_info_struct failed");

        if (setjmp(png_jmpbuf(pngfile.png_ptr)))
        	throw logic_error("Error during init_io");

        png_init_io(pngfile.png_ptr, fp);

        // header
        if (setjmp(png_jmpbuf(pngfile.png_ptr)))
        	throw logic_error("Error during writing header");

        png_set_IHDR(pngfile.png_ptr, pngfile.info_ptr, pngfile.width, pngfile.height,
        		pngfile.bit_depth, pngfile.color_type, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        png_write_info(pngfile.png_ptr, pngfile.info_ptr);

        // write
        if (setjmp(png_jmpbuf(pngfile.png_ptr)))
        	throw logic_error("Error during writing bytes");

        png_write_image(pngfile.png_ptr, pngfile.row_ptrs.data());

        if (setjmp(png_jmpbuf(pngfile.png_ptr)))
        	throw logic_error("Error during end of write");

        png_write_end(pngfile.png_ptr, NULL);

        fclose(fp);
}
