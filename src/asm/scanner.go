package asm

import (
	"bytes"
	"fmt"
	"io/ioutil"
	"math"
	"os"
	"strconv"
	"unicode"
	"unicode/utf8"
)

type Pos struct {
	Line, Col int
}

func (p Pos) String() string {
	return fmt.Sprintf("(%d, %d)", p.Line, p.Col)
}

type Token struct {
	File   *File
	Pos    Pos
	Kind   int
	Text   string
	IntVal uint64
	StrVal string
}

const (
	TokEOF = 0
	TokReg = 128 + iota
	TokDirective
	TokName
	TokInt
	TokString
)

func kindName(kind int) string {
	switch kind {
	case TokReg:
		return "reg"

	case TokDirective:
		return "directive"

	case TokName:
		return "name"

	case TokInt:
		return "int"

	case TokString:
		return "string"

	default:
		return string(kind)
	}
}

func (t *Token) String() (str string) {
	switch {
	case t.Kind == TokEOF:
		str = fmt.Sprintf("token(eof) %s", t.Pos)

	case t.Kind == TokInt:
		str = fmt.Sprintf("token(int) %s '%s' %d", t.Pos, t.Text, t.IntVal)

	case t.Kind > 127:
		str = fmt.Sprintf("token(%s) %s '%.*s'", kindName(t.Kind), t.Pos, 32, t.Text)

	default:
		str = fmt.Sprintf("token('%s') %s", kindName(t.Kind), t.Pos)
	}
	return
}

type Scanner struct {
	File  *File
	Data  []rune
	pos   int
	Pos   Pos
	Token *Token
}

func Scan(file *File) *Scanner {
	bytes, err := ioutil.ReadFile(file.Path)
	if err != nil {
		fmt.Printf("error: %s\n", err.Error())
		os.Exit(1)
	}

	return &Scanner{
		File:  file,
		Data:  []rune(string(bytes)),
		pos:   0,
		Pos:   Pos{1, 1},
		Token: nil,
	}
}

func (s *Scanner) errorf(msg string, args ...interface{}) {
	s.File.Errorf(s.Pos, msg, args...)
}

func (s *Scanner) chr() rune {
	if s.eof() {
		return 0
	}
	return s.Data[s.pos]
}

func (s *Scanner) forward() {
	if s.Token != nil {
		s.Token.Text += string(s.chr())
	}

	s.pos++
	s.Pos.Col++

	if s.chr() == '\n' {
		s.Pos.Line++
		s.Pos.Col = 0
	}
}

func (s *Scanner) match(r rune) bool {
	if s.chr() == r {
		s.forward()
		return true
	}

	return false
}

func (s *Scanner) fmatch(f func(rune) bool) bool {
	if f(s.chr()) {
		s.forward()
		return true
	}

	return false
}

func (s *Scanner) expect(r rune) bool {
	if !s.match(r) {
		s.errorf("expected '%c', got '%c'", r, s.chr())
		return false
	}

	return true
}

func (s *Scanner) fexpect(f func(rune) bool) bool {
	if !s.fmatch(f) {
		s.errorf("unexpected character '%c'", s.chr())
		return false
	}

	return true
}

func (s *Scanner) eof() bool {
	return s.pos >= len(s.Data)
}

func isHex(r rune) bool {
	return unicode.IsDigit(r) || (r >= 'a' && r <= 'f') || (r >= 'A' && r <= 'F')
}

func isNameStart(r rune) bool {
	return unicode.IsLetter(r) || r == '_'
}

func isNamePart(r rune) bool {
	return isNameStart(r) || unicode.IsDigit(r) || r == '.'
}

func (s *Scanner) newTok() {
	s.Token = &Token{
		File: s.File,
		Pos:  s.Pos,
	}
}

func (s *Scanner) scanName() {
	s.Token.Kind = TokName
	for s.fmatch(isNamePart) {
	}
}

var charIntVal = []uint64{
	'0': 0,
	'1': 1,
	'2': 2,
	'3': 3,
	'4': 4,
	'5': 5,
	'6': 6,
	'7': 7,
	'8': 8,
	'9': 9,
	'a': 10, 'A': 10,
	'b': 11, 'B': 11,
	'c': 12, 'C': 12,
	'd': 13, 'D': 13,
	'e': 14, 'E': 14,
	'f': 15, 'F': 15,
}

func (s *Scanner) scanInt() {
	t := s.Token
	t.Kind = TokInt

	var base uint64 = 10

	if s.match('0') {
		switch {
		case s.match('x') || s.match('X'):
			base = 16

		case s.match('b') || s.match('B'):
			base = 2

		default:
			base = 8
		}
	}

	for {
		if s.match('_') {
			continue
		}

		digit := charIntVal[s.chr()]
		if digit == 0 && s.chr() != '0' {
			break
		}

		if digit >= base {
			s.errorf("digit '%c' out of range for base %d", s.chr(), base)
			digit = 0
		}

		if t.IntVal > math.MaxUint64-digit/base {
			s.errorf("integer literal overflow (%d)", t.IntVal)
			for s.fmatch(isHex) || s.match('_') {
			}

			t.IntVal = 0
			break
		}

		t.IntVal = t.IntVal*base + digit
		s.forward()
	}
}

var regVal = map[string]uint32{
	"%zero": 0, "%at": 1, "%v0": 2, "%v1": 3,
	"%a0": 4, "%a1": 5, "%a2": 6, "%a3": 7,
	"%t0": 8, "%t1": 9, "%t2": 10, "%t3": 11,
	"%t4": 12, "%t5": 13, "%t6": 14, "%t7": 15,
	"%s0": 16, "%s1": 17, "%s2": 18, "%s3": 19,
	"%s4": 20, "%s5": 21, "%s6": 22, "%s7": 23,
	"%t8": 24, "%t9": 25, "%k0": 26, "%k1": 27,
	"%gp": 28, "%sp": 29, "%fp": 30, "%ra": 31,
}

func (s *Scanner) scanRegister() {
	t := s.Token
	t.Kind = TokReg
	for s.fmatch(isNamePart) {
	}

	if IntVal, ok := regVal[t.Text]; ok {
		t.IntVal = uint64(IntVal)
	} else {
		s.errorf("unknown register '%s'", t.Text)
	}
}

func (s *Scanner) scanEscape() bool {
	t := s.Token
	switch {
	case s.match('n'):
		t.IntVal = '\n'
		t.StrVal += "\n"

	case s.match('r'):
		t.IntVal = '\r'
		t.StrVal += "\r"

	case s.match('t'):
		t.IntVal = '\t'
		t.StrVal += "\t"

	case s.match('\\'):
		t.IntVal = '\\'
		t.StrVal += "\\"

	case s.match('\''):
		t.IntVal = '\''
		t.StrVal += "'"

	case s.match('"'):
		t.IntVal = '"'
		t.StrVal += "\""

	case s.match('0'):
		t.IntVal = 0
		t.StrVal += "\\0"

	case s.match('x'): // hexadecimal
		s.fexpect(isHex)
		s.fmatch(isHex)

		esc := t.Text[3:]
		n, err := strconv.ParseInt(esc, 16, 0)
		if err != nil {
			t.StrVal += "\\" + esc
			return false
		}
		t.StrVal += string(n)
		t.IntVal = uint64(n)

	case s.match('u'): // unicode
		s.fexpect(isHex)
		s.fmatch(isHex)
		s.fmatch(isHex)
		s.fmatch(isHex)

		esc := t.Text[3:]
		n, _ := strconv.ParseInt(esc, 16, 0)
		u := make([]byte, 4)
		utf8.EncodeRune(u, rune(n))
		b := bytes.Trim(u, "\x00")
		t.StrVal += string(b) // remove NULL bytes
		t.IntVal = uint64(n)

	default:
		s.errorf("unexpected character in escape sequence '%c'", s.chr())
		return false
	}
	return true
}

func (s *Scanner) scanChar() {
	t := s.Token
	t.Kind = TokInt

	switch {
	case s.match('\\'):
		s.scanEscape()
		break

	default:
		t.IntVal = uint64(s.chr())
		s.forward()
	}
	s.expect('\'')
}

func (s *Scanner) scanString() {
	t := s.Token
	t.Kind = TokString

	for !s.eof() {
		switch {
		case s.eof() || s.match('\n'):
			s.errorf("unclosed string literal")
			return

		case s.fmatch(unicode.IsControl):
			s.errorf("illegal control character in string literal")

		case s.match('"'):
			return

		case s.match('\\'):
			s.scanEscape()

		default:
			t.StrVal += string(s.chr())
			s.forward()
		}
	}
}

func (s *Scanner) discardSpaces() {
	for s.fmatch(unicode.IsSpace) {
	}
}

func (s *Scanner) discardComment() {
	for !s.match('\n') {
		s.forward()
	}
}

/*
func (s *Scanner) discardBlockComment() bool {
	for depth := 1; depth > 0; {
		switch {
		case s.eof():
			s.errorf("unclosed block comment")
			return false

		case s.match('/') && s.match('*'):
			depth++

		case s.match('*') && s.match('/'):
			depth--

		default:
			s.forward()
		}
	}
	return true
}
*/

func (s *Scanner) Tokenize() {
	s.discardSpaces()

	s.newTok()
	t := s.Token

	switch {
	case s.eof():
		t.Kind = TokEOF
		return

	case s.fmatch(isNameStart):
		s.scanName()
		return

	case unicode.IsDigit(s.chr()):
		s.scanInt()
		return

	case s.match('%'):
		s.scanRegister()
		return

	case s.match('"'):
		s.scanString()
		return

	case s.match('\''):
		s.scanChar()
		return

	case s.match(';'):
		switch {
		case s.match(';'):
			s.discardComment()

		default:
			t.Kind = int(s.chr())
			s.forward()
			return
		}

		s.Tokenize()
		return

	case s.match('.'):
		s.scanName()
		t.Kind = TokDirective
		return

	default:
		t.Kind = int(s.chr())
		s.forward()
		return
	}
}

func safeRune(r rune) string {
	switch r {
	case '\n':
		return "\\n"

	case '\r':
		return "\\r"

	case '\t':
		return "\\t"

	case '\'':
		return "\\'"

	case '"':
		return "\\\""

	default:
		if unicode.IsPrint(r) {
			return string(r)
		}
		return fmt.Sprintf("\\%x", r)
	}
}
