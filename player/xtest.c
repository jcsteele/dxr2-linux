#include "XOverlay.h"
#include "types.h"

int main(int argc, char** argv)
{
  geom_t geom;

  geom.x=10;
  geom.y=10;
  geom.width=100;
  geom.height=100;

  init_win(&argc, argv, geom);
  while(1);
}
