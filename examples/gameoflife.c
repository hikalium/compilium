void putchar(char c);
void usleep(int us);

int main() {
  int x;
  int y;
  int p;
  int q;
  int p;
  int col;
  int count;
  int map[32][32];
  int size = 32;
  int mask = size - 1;

  for (y = 0; y < size; y++) {
    for (x = 0; x < size; x++) {
      map[y][x] = 0;
    }
  }

  int cx = size / 2;
  int cy = size / 2;

  map[cy - 1][cx - 3] = 1;
  map[cy - 1][cx + 2] = 1;

  map[cy][cx - 4] = 1;
  map[cy][cx - 3] = 1;
  map[cy][cx + 2] = 1;
  map[cy][cx + 3] = 1;

  map[cy + 1][cx - 3] = 1;
  map[cy + 1][cx + 2] = 1;

  for (1; 1; 1) {
    for (y = 0; y < size; y++) {
      for (x = 0; x < size; x++) {
        count = 0;
        for (p = -1; p <= 1; p++)
          for (q = -1; q <= 1; q++)
            count += map[(y + p) & mask][(x + q) & mask] & 1;
        count -= map[y][x] & 1;
        if ((map[y][x] && (count == 2 || count == 3)) ||
            (!map[y][x] && count == 3))
          map[y][x] = map[y][x] | 2;
      }
    }
    for (y = 0; y < size; y++) {
      for (x = 0; x < size; x++) {
        map[y][x] = map[y][x] >> 1;
        if (map[y][x])
          putchar('*');
        else
          putchar(' ');
        putchar(' ');
      }
      putchar('\n');
    }
    usleep(1000 * 100 * 5);
    putchar('\n');
  }
}
