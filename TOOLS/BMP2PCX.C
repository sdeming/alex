// bmp2pcx by Scott Deming
//
// short but sweet
//
// compile: gcc bmp2pcx.c -o bmp2pcx.exe -lalleg
// run: bmp2pcx file.bmp file.pcx

#include <allegro.h>

int main(int argc, char* argv[])
{
	PALETTE pal;
	BITMAP *bmp;

	allegro_init();

	bmp = load_bmp(argv[1], pal);
	save_pcx(argv[2], bmp, pal);
	destroy_bitmap(bmp);
}
