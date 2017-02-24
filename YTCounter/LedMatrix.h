#define CMD_NOOP         0
#define CMD_DIGIT0       1
#define CMD_DIGIT1       2
#define CMD_DIGIT2       3
#define CMD_DIGIT3       4
#define CMD_DIGIT4       5
#define CMD_DIGIT5       6
#define CMD_DIGIT6       7
#define CMD_DIGIT7       8
#define CMD_DECODEMODE   9
#define CMD_INTENSITY    10
#define CMD_SCANLIMIT    11
#define CMD_SHUTDOWN     12
#define CMD_DISPLAYTEST  15

byte buffer[NUMMAX * 8 + 8];

uint8_t dualChar = 0;

void sendCmd(byte cmd, byte data)
{
  digitalWrite(CSPin, LOW);
  for (int8_t i = NUMMAX - 1; i >= 0; i--)
  {
    shiftOut(DINPin, CLKPin, MSBFIRST, cmd);
    shiftOut(DINPin, CLKPin, MSBFIRST, data);
  }
  digitalWrite(CSPin, HIGH);
}

void refreshMatrix()
{
#if ROTATE == 270
  byte mask = 0x01;
  for (uint8_t c = 0; c < 8; c++)
  {
    digitalWrite(CSPin, LOW);
    for (int8_t i = NUMMAX - 1; i >= 0; i--)
    {
      byte bt = 0;
      for (uint8_t b = 0; b < 8; b++)
      {
        bt <<= 1;
        if (buffer[i * 8 + b] & mask) bt |= 0x01;
      }
      shiftOut(DINPin, CLKPin, MSBFIRST, CMD_DIGIT0 + c);
      shiftOut(DINPin, CLKPin, MSBFIRST, bt);
    }
    digitalWrite(CSPin, HIGH);
    mask <<= 1;
  }
#elif ROTATE == 90
  byte mask = 0x80;
  for (uint8_t c = 0; c < 8; c++)
  {
    digitalWrite(CSPin, LOW);
    for (int8_t i = NUMMAX - 1; i >= 0; i--)
    {
      byte bt = 0;
      for (uint8_t b = 0; b < 8; b++)
      {
        bt >>= 1;
        if (buffer[i * 8 + b] & mask) bt |= 0x80;
      }
      shiftOut(DINPin, CLKPin, MSBFIRST, CMD_DIGIT0 + c);
      shiftOut(DINPin, CLKPin, MSBFIRST, bt);
    }
    digitalWrite(CSPin, HIGH);
    mask >>= 1;
  }
#else
  for (uint8_t c = 0; c < 8; c++)
  {
    digitalWrite(CSPin, LOW);
    for (int8_t i = NUMMAX - 1; i >= 0; i--)
    {
      shiftOut(DINPin, CLKPin, MSBFIRST, CMD_DIGIT0 + c);
      shiftOut(DINPin, CLKPin, MSBFIRST, buffer[i * 8 + c]);
    }
    digitalWrite(CSPin, HIGH);
  }
#endif
}

void clearMatrix()
{
  for (uint8_t i = 0; i < NUMMAX * 8; i++) buffer[i] = 0;
  refreshMatrix();
}

void initMatrix()
{
  pinMode(DINPin, OUTPUT);
  pinMode(CLKPin, OUTPUT);
  pinMode(CSPin, OUTPUT);
  digitalWrite(CSPin, HIGH);
  sendCmd(CMD_DISPLAYTEST, 0);
  sendCmd(CMD_SCANLIMIT, 7);
  sendCmd(CMD_DECODEMODE, 0);
  sendCmd(CMD_INTENSITY, 0);
  sendCmd(CMD_SHUTDOWN, 1);
  clearMatrix();
}

void setMatrixIntensity(byte val)
{
  sendCmd(CMD_INTENSITY, val);
}

unsigned char convertTurkish(unsigned char chr)
{
  unsigned char ch = chr;

  if (ch == 195 || ch == 196 || ch == 197)
  {
    dualChar = ch;
    return 0;
  }

  if(dualChar)
  {
    switch(chr)
    {
      case 135: ch = 128; break; // Ç
      case 150: ch = 129; break; // Ö
      case 156: ch = 133; break; // Ü
      case 158: ch = dualChar == 196 ? 132 : 130; break; // Ğ and Ş
      case 159: ch = dualChar == 196 ? 138 : 136; break; // ğ and ş
      case 167: ch = 134; break; // ç
      case 176: ch = 131; break; // İ
      case 177: ch = 137; break; // ı
      case 182: ch = 135; break; // ö
      case 188: ch = 139; break; // ü
      default:  break;
    }
    dualChar = 0;
  }

  return ch;
}

uint8_t charWidth(unsigned char chr, const uint8_t *font)
{
  uint8_t len = pgm_read_byte(font);
  uint8_t firstChar = pgm_read_byte(font + 1);
  unsigned char ch = convertTurkish(chr + firstChar);
  if (ch < firstChar) return -1;
  ch -= firstChar;
  return pgm_read_byte(font + 4 + ch * len);
}

uint8_t stringWidth(const char *str, const uint8_t *font)
{
  uint8_t strWidth = 0;
  uint8_t firstChar = pgm_read_byte(font + 1);
  while (*str) strWidth += charWidth(*str++ - firstChar, font) + 1;
  strWidth--;
  return strWidth;
}

uint8_t showChar(unsigned char chr, const uint8_t *font)
{
  uint8_t len = pgm_read_byte(font);
  uint8_t width = pgm_read_byte(font + 4 + chr * len);
  buffer[NUMMAX * 8] = 0;
  for (uint8_t i = 0; i < width; i++) buffer[NUMMAX * 8 + i + 1] = pgm_read_byte(font + 4 + chr * len + 1 + i);
  return width;
}

void scrollMatrix()
{
  for (uint8_t i = 0; i < NUMMAX * 8 + 7; i++) buffer[i] = buffer[i + 1];
}

void scrollChar(unsigned char chr, const uint8_t *font, uint8_t scrollDelay)
{
  unsigned char ch = convertTurkish(chr);
  uint8_t firstChar = pgm_read_byte(font + 1);
  uint8_t lastChar = pgm_read_byte(font + 2);
  if (ch < firstChar || ch > lastChar) return;
  ch -= firstChar;
  uint8_t width = showChar(ch, font);
  for (uint8_t i = 0; i < width + 1; i++)
  {
    delay(scrollDelay);
    scrollMatrix();
    refreshMatrix();
  }
}

void scrollString(const char *str, const uint8_t *font, uint8_t scrollDelay)
{
  while (*str) scrollChar(*str++, font, scrollDelay);
}

void scrollStringCenter(const char *str, const uint8_t *font, uint8_t scrollDelay)
{
  uint8_t strWidth = stringWidth(str, font);
  uint8_t emptyLeft = (NUMMAX * 8 - strWidth) / 2;
  uint8_t emptyRight = NUMMAX * 8 - emptyLeft - strWidth;
  uint8_t emptyChar = pgm_read_byte(font + 3);
  for (uint8_t i = 0; i < emptyLeft; i++) scrollChar(emptyChar, font, scrollDelay);
  scrollString(str, font, scrollDelay);
  for (uint8_t i = 0; i < emptyRight; i++) scrollChar(emptyChar, font, scrollDelay);
}

uint8_t printChar(unsigned char chr, const uint8_t *font, uint8_t pos)
{
  unsigned char ch = convertTurkish(chr);
  uint8_t firstChar = pgm_read_byte(font + 1);
  uint8_t lastChar = pgm_read_byte(font + 2);
  if (ch < firstChar || ch > lastChar) return -1;
  ch -= firstChar;
  uint8_t len = pgm_read_byte(font);
  uint8_t width = pgm_read_byte(font + 4 + ch * len);
  for (uint8_t i = pos; i < pos + width; i++) buffer[i] = pgm_read_byte(font + 4 + ch * len + 1 + i - pos);
  refreshMatrix();
  return width;
}

void printString(const char *s, const uint8_t *font, uint8_t pos)
{
  uint8_t charPos = pos;
  while (*s)
  {
    uint8_t width = printChar(*s++, font, charPos);
    charPos += width + 1;
  }
}

void printStringCenter(const char *str, const uint8_t *font)
{
  uint8_t strWidth = stringWidth(str, font);
  uint8_t pos = (NUMMAX * 8 - strWidth) / 2;
  printString(str, font, pos);
}

void blinkStringCenter(const char *str, const uint8_t *font, uint8_t blinkDelay, uint8_t blinkCount)
{
  for (uint8_t i = 0; i < blinkCount; i++)
  {
    delay(blinkDelay);
    clearMatrix();
    delay(blinkDelay);
    printStringCenter(str, font);
  }
}

char *formatNumber(long val)
{
  char *input = (char *) malloc(sizeof(long));
  ltoa(val, input, 10);
  uint8_t inlen = strlen(input);
  uint8_t commas = inlen / 3;
  if (inlen % 3 == 0) commas--;
  uint8_t outlen = inlen + commas + 1;
  char *output = (char *) malloc(outlen);
  memset(output, 0, outlen);
  int8_t outpos = outlen - 2;
  int8_t inpos = inlen - 1;
  uint8_t i = 0;
  while (outpos >= 0)
  {
    output[outpos--] = input[inpos--];
    i++;
    if (i % 3 == 0) output[outpos--] = '.';
  }
  free(input);
  return output;
}
