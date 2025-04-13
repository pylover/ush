#ifndef ASCII_H_
#define ASCII_H_


/* control characters */
#define ASCII_LF 10
#define ASCII_ESC 27


/** idf monitor
 * backspace: 8
 * delete: 27[126
 */

/** screen
 * backspace: 127
 * delete: 27[126
 */
#define ASCII_ISBACKSPACE(c) ((c == 8) || (c == 127))
#define ASCII_ISDIGIT(c) ((c >= 48) && (c <= 57))
#define ASCII_IS1TO9(c) ((c >= 49) && (c <= 57))
#define ASCII_ISALPHA(c) ( \
        ((c >= 65) && (c <= 90)) || \
        ((c >= 97) && (c <= 122)))
#define ASCII_ISBLANK(c) ((c == 9) || (c <= 32))
#define ASCII_ISWORDSEP(c) (!(ASCII_ISDIGIT(c) || ASCII_ISALPHA(c)))


/** ASCII control characters (character code 0-31)
 * The first 32 characters in the ASCII-table are unprintable control codes
 * and are used to control peripherals such as printers.

DEC OCT  HEX BIN       Symb Description
0   000  00  00000000  NUL  Null character
1   001  01  00000001  SOH  Start of Heading
2   002  02  00000010  STX  Start of Text
3   003  03  00000011  ETX  End of Text
4   004  04  00000100  EOT  End of Transmission
5   005  05  00000101  ENQ  Enquiry
6   006  06  00000110  ACK  Acknowledge
7   007  07  00000111  BEL  Bell, Alert
8   010  08  00001000  BS   Backspace
9   011  09  00001001  HT   Horizontal Tab
10  012  0A  00001010  LF   Line Feed
11  013  0B  00001011  VT   Vertical Tabulation
12  014  0C  00001100  FF   Form Feed
13  015  0D  00001101  CR   Carriage Return
14  016  0E  00001110  SO   Shift Out
15  017  0F  00001111  SI   Shift In
16  020  10  00010000  DLE  Data Link Escape
17  021  11  00010001  DC1  Device Control One (XON)
18  022  12  00010010  DC2  Device Control Two
19  023  13  00010011  DC3  Device Control Three (XOFF)
20  024  14  00010100  DC4  Device Control Four
21  025  15  00010101  NAK  Negative Acknowledge
22  026  16  00010110  SYN  Synchronous Idle
23  027  17  00010111  ETB  End of Transmission Block
24  030  18  00011000  CAN  Cancel
25  031  19  00011001  EM   End of medium
26  032  1A  00011010  SUB  Substitute
27  033  1B  00011011  ESC  Escape
28  034  1C  00011100  FS   File Separator
29  035  1D  00011101  GS   Group Separator
30  036  1E  00011110  RS   Record Separator
31  037  1F  00011111  US   Unit Separator
*/

/** ASCII printable characters (character code 32-127)
 * Codes 32-127 are common for all the different variations of the ASCII
 * table, they are called printable characters, represent letters, digits,
 * punctuation marks, and a few miscellaneous symbols. You will find almost
 * every character on your keyboard. Character 127 represents the command DEL.

DEC OCT  HEX BIN       Symb Description
32  040  20  00100000  SP   Space
33  041  21  00100001  !    Exclamation mark
34  042  22  00100010  "    Double quotes (or speech marks)
35  043  23  00100011  #    Number sign
36  044  24  00100100  $    Dollar
37  045  25  00100101  %    Per cent sign
38  046  26  00100110  &    Ampersand
39  047  27  00100111  '    Single quote
40  050  28  00101000  (    Open parenthesis (or open bracket)
41  051  29  00101001  )    Close parenthesis (or close bracket)
42  052  2A  00101010  *    Asterisk
43  053  2B  00101011  +    Plus
44  054  2C  00101100  ,    Comma
45  055  2D  00101101  -    Hyphen-minus
46  056  2E  00101110  .    Period, dot or full stop
47  057  2F  00101111  /    Slash or divide
48  060  30  00110000  0    Zero
49  061  31  00110001  1    One
50  062  32  00110010  2    Two
51  063  33  00110011  3    Three
52  064  34  00110100  4    Four
53  065  35  00110101  5    Five
54  066  36  00110110  6    Six
55  067  37  00110111  7    Seven
56  070  38  00111000  8    Eight
57  071  39  00111001  9    Nine
58  072  3A  00111010  :    Colon
59  073  3B  00111011  ;    Semicolon
60  074  3C  00111100  <    Less than (or open angled bracket)
61  075  3D  00111101  =    Equals
62  076  3E  00111110  >    Greater than (or close angled bracket)
63  077  3F  00111111  ?    Question mark
64  100  40  01000000  @    At sign
65  101  41  01000001  A    Uppercase A
66  102  42  01000010  B    Uppercase B
67  103  43  01000011  C    Uppercase C
68  104  44  01000100  D    Uppercase D
69  105  45  01000101  E    Uppercase E
70  106  46  01000110  F    Uppercase F
71  107  47  01000111  G    Uppercase G
72  110  48  01001000  H    Uppercase H
73  111  49  01001001  I    Uppercase I
74  112  4A  01001010  J    Uppercase J
75  113  4B  01001011  K    Uppercase K
76  114  4C  01001100  L    Uppercase L
77  115  4D  01001101  M    Uppercase M
78  116  4E  01001110  N    Uppercase N
79  117  4F  01001111  O    Uppercase O
80  120  50  01010000  P    Uppercase P
81  121  51  01010001  Q    Uppercase Q
82  122  52  01010010  R    Uppercase R
83  123  53  01010011  S    Uppercase S
84  124  54  01010100  T    Uppercase T
85  125  55  01010101  U    Uppercase U
86  126  56  01010110  V    Uppercase V
87  127  57  01010111  W    Uppercase W
88  130  58  01011000  X    Uppercase X
89  131  59  01011001  Y    Uppercase Y
90  132  5A  01011010  Z    Uppercase Z
91  133  5B  01011011  [    Opening bracket
92  134  5C  01011100  \    Backslash
93  135  5D  01011101  ]    Closing bracket
94  136  5E  01011110  ^    Caret - circumflex
95  137  5F  01011111  _    Underscore
96  140  60  01100000  `    Grave accent
97  141  61  01100001  a    Lowercase a
98  142  62  01100010  b    Lowercase b
99  143  63  01100011  c    Lowercase c
100 144  64  01100100  d    Lowercase d
101 145  65  01100101  e    Lowercase e
102 146  66  01100110  f    Lowercase f
103 147  67  01100111  g    Lowercase g
104 150  68  01101000  h    Lowercase h
105 151  69  01101001  i    Lowercase i
106 152  6A  01101010  j    Lowercase j
107 153  6B  01101011  k    Lowercase k
108 154  6C  01101100  l    Lowercase l
109 155  6D  01101101  m    Lowercase m
110 156  6E  01101110  n    Lowercase n
111 157  6F  01101111  o    Lowercase o
112 160  70  01110000  p    Lowercase p
113 161  71  01110001  q    Lowercase q
114 162  72  01110010  r    Lowercase r
115 163  73  01110011  s    Lowercase s
116 164  74  01110100  t    Lowercase t
117 165  75  01110101  u    Lowercase u
118 166  76  01110110  v    Lowercase v
119 167  77  01110111  w    Lowercase w
120 170  78  01111000  x    Lowercase x
121 171  79  01111001  y    Lowercase y
122 172  7A  01111010  z    Lowercase z
123 173  7B  01111011  {    Opening brace
124 174  7C  01111100  |    Vertical bar
125 175  7D  01111101  }    Closing brace
126 176  7E  01111110  ~    Equivalency sign - tilde
127 177  7F  01111111  DEL  Delete
*/



/** The extended ASCII codes (character code 128-255)
 * There are several different variations of the 8-bit ASCII table. The table
 * below is according to Windows-1252 (CP-1252) which is a superset of ISO
 * 8859-1, also called ISO Latin-1, in terms of printable characters, but
 * differs from the IANA's ISO-8859-1 by using displayable characters rather
 * than control characters in the 128 to 159 range. Characters that differ
 * from ISO-8859-1 is marked by light blue color.

DEC OCT  HEX BIN       Symb Description
128 200  80  10000000  €    Euro sign
129 201  81  10000001       Unused
130 202  82  10000010  ‚    Single low-9 quotation mark
131 203  83  10000011  ƒ    Latin small letter f with hook
132 204  84  10000100  „    Double low-9 quotation mark
133 205  85  10000101  …    Horizontal ellipsis
134 206  86  10000110  †    Dagger
135 207  87  10000111  ‡    Double dagger
136 210  88  10001000  ˆ    Modifier letter circumflex accent
137 211  89  10001001  ‰    Per mille sign
138 212  8A  10001010  Š    Latin capital letter S with caron
139 213  8B  10001011  ‹    Single left-pointing angle quotation
140 214  8C  10001100  Œ    Latin capital ligature OE
141 215  8D  10001101       Unused
142 216  8E  10001110  Ž    Latin capital letter Z with caron
143 217  8F  10001111       Unused
144 220  90  10010000       Unused
145 221  91  10010001  ‘    Left single quotation mark
146 222  92  10010010  ’    Right single quotation mark
147 223  93  10010011  “    Left double quotation mark
148 224  94  10010100  ”    Right double quotation mark
149 225  95  10010101  •    Bullet
150 226  96  10010110  –    En dash
151 227  97  10010111  —    Em dash
152 230  98  10011000  ˜    Small tilde
153 231  99  10011001  ™    Trade mark sign
154 232  9A  10011010  š    Latin small letter S with caron
155 233  9B  10011011  ›    Single right-pointing angle quotation mark
156 234  9C  10011100  œ    Latin small ligature oe
157 235  9D  10011101       Unused
158 236  9E  10011110  ž    Latin small letter z with caron
159 237  9F  10011111  Ÿ    Latin capital letter Y with diaeresis
160 240  A0  10100000  NBSP Non-breaking space
161 241  A1  10100001  ¡    Inverted exclamation mark
162 242  A2  10100010  ¢    Cent sign
163 243  A3  10100011  £    Pound sign
164 244  A4  10100100  ¤    Currency sign
165 245  A5  10100101  ¥    Yen sign
166 246  A6  10100110  ¦    Pipe, broken vertical bar
167 247  A7  10100111  §    Section sign
168 250  A8  10101000  ¨    Spacing diaeresis - umlaut
169 251  A9  10101001  ©    Copyright sign
170 252  AA  10101010  ª    Feminine ordinal indicator
171 253  AB  10101011  «    Left double angle quotes
172 254  AC  10101100  ¬    Negation
173 255  AD  10101101  ­SHY Soft hyphen
174 256  AE  10101110  ®    Registered trade mark sign
175 257  AF  10101111  ¯    Spacing macron - overline
176 260  B0  10110000  °    Degree sign
177 261  B1  10110001  ±    Plus-or-minus sign
178 262  B2  10110010  ²    Superscript two - squared
179 263  B3  10110011  ³    Superscript three - cubed
180 264  B4  10110100  ´    Acute accent - spacing acute
181 265  B5  10110101  µ    Micro sign
182 266  B6  10110110  ¶    Pilcrow sign - paragraph sign
183 267  B7  10110111  ·    Middle dot - Georgian comma
184 270  B8  10111000  ¸    Spacing cedilla
185 271  B9  10111001  ¹    Superscript one
186 272  BA  10111010  º    Masculine ordinal indicator
187 273  BB  10111011  »    Right double angle quotes
188 274  BC  10111100  ¼    Fraction one quarter
189 275  BD  10111101  ½    Fraction one half
190 276  BE  10111110  ¾    Fraction three quarters
191 277  BF  10111111  ¿    Inverted question mark
192 300  C0  11000000  À    Latin capital letter A with grave
193 301  C1  11000001  Á    Latin capital letter A with acute
194 302  C2  11000010  Â    Latin capital letter A with circumflex
195 303  C3  11000011  Ã    Latin capital letter A with tilde
196 304  C4  11000100  Ä    Latin capital letter A with diaeresis
197 305  C5  11000101  Å    Latin capital letter A with ring above
198 306  C6  11000110  Æ    Latin capital letter AE
199 307  C7  11000111  Ç    Latin capital letter C with cedilla
200 310  C8  11001000  È    Latin capital letter E with grave
201 311  C9  11001001  É    Latin capital letter E with acute
202 312  CA  11001010  Ê    Latin capital letter E with circumflex
203 313  CB  11001011  Ë    Latin capital letter E with diaeresis
204 314  CC  11001100  Ì    Latin capital letter I with grave
205 315  CD  11001101  Í    Latin capital letter I with acute
206 316  CE  11001110  Î    Latin capital letter I with circumflex
207 317  CF  11001111  Ï    Latin capital letter I with diaeresis
208 320  D0  11010000  Ð    Latin capital letter ETH
209 321  D1  11010001  Ñ    Latin capital letter N with tilde
210 322  D2  11010010  Ò    Latin capital letter O with grave
211 323  D3  11010011  Ó    Latin capital letter O with acute
212 324  D4  11010100  Ô    Latin capital letter O with circumflex
213 325  D5  11010101  Õ    Latin capital letter O with tilde
214 326  D6  11010110  Ö    Latin capital letter O with diaeresis
215 327  D7  11010111  ×    Multiplication sign
216 330  D8  11011000  Ø    Latin capital letter O with slash
217 331  D9  11011001  Ù    Latin capital letter U with grave
218 332  DA  11011010  Ú    Latin capital letter U with acute
219 333  DB  11011011  Û    Latin capital letter U with circumflex
220 334  DC  11011100  Ü    Latin capital letter U with diaeresis
221 335  DD  11011101  Ý    Latin capital letter Y with acute
222 336  DE  11011110  Þ    Latin capital letter THORN
223 337  DF  11011111  ß    Latin small letter sharp s - ess-zed
224 340  E0  11100000  à    Latin small letter a with grave
225 341  E1  11100001  á    Latin small letter a with acute
226 342  E2  11100010  â    Latin small letter a with circumflex
227 343  E3  11100011  ã    Latin small letter a with tilde
228 344  E4  11100100  ä    Latin small letter a with diaeresis
229 345  E5  11100101  å    Latin small letter a with ring above
230 346  E6  11100110  æ    Latin small letter ae
231 347  E7  11100111  ç    Latin small letter c with cedilla
232 350  E8  11101000  è    Latin small letter e with grave
233 351  E9  11101001  é    Latin small letter e with acute
234 352  EA  11101010  ê    Latin small letter e with circumflex
235 353  EB  11101011  ë    Latin small letter e with diaeresis
236 354  EC  11101100  ì    Latin small letter i with grave
237 355  ED  11101101  í    Latin small letter i with acute
238 356  EE  11101110  î    Latin small letter i with circumflex
239 357  EF  11101111  ï    Latin small letter i with diaeresis
240 360  F0  11110000  ð    Latin small letter eth
241 361  F1  11110001  ñ    Latin small letter n with tilde
242 362  F2  11110010  ò    Latin small letter o with grave
243 363  F3  11110011  ó    Latin small letter o with acute
244 364  F4  11110100  ô    Latin small letter o with circumflex
245 365  F5  11110101  õ    Latin small letter o with tilde
246 366  F6  11110110  ö    Latin small letter o with diaeresis
247 367  F7  11110111  ÷    Division sign
248 370  F8  11111000  ø    Latin small letter o with slash
249 371  F9  11111001  ù    Latin small letter u with grave
250 372  FA  11111010  ú    Latin small letter u with acute
251 373  FB  11111011  û    Latin small letter u with circumflex
252 374  FC  11111100  ü    Latin small letter u with diaeresis
253 375  FD  11111101  ý    Latin small letter y with acute
254 376  FE  11111110  þ    Latin small letter thorn
255 377  FF  11111111  ÿ    Latin small letter y with diaeresis
*/


#endif  // ASCII_H_
