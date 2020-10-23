int write(int fd, const void *, int);
void exit(int);

void printf04d(int v) {
  char s[4];
  for (int i = 0; i < 4; i++) {
    s[3 - i] = v % 10 + '0';
    v /= 10;
  }
  write(1, s, 4);
}

int nume[52514];
int i;
int n;
int carry;
int digit;
int base;
int denom;
int first;

int main(int argc, char **argv) {
  base = 10000;
  for (n = 52500; n > 0; n -= 14) {
    carry %= base;
    digit = carry;
    for (i = n - 1; i > 0; --i) {
      denom = 2 * i - 1;
      carry = carry * i + base * (first ? nume[i] : (base / 5));
      nume[i] = carry % denom;
      carry /= denom;
    }
    printf04d(digit + carry / base);
    first = 1;
  }
  write(1, "\n", 1);
  exit(0);
  return 0;
}
