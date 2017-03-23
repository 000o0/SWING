#ifndef _SWING_TOKEN_H_
#define _SWING_TOKEN_H_

#include <list>
#include <string>

enum class TokenID
{
	Eof = 0,

	Unknown,

	Comma = ',',
	Assignment = '=',
	Quotmark = '"',
	Plus = '+',
	Minus = '-',
	Multi = '*',
	Divide = '/',
	Mod = '%',
	And = '&',
	Or = '|',
	Less = '<',
	Great = '>',
	Not = '!',
	OpenSmall = '(',
	CloseSmall = ')',
	OpenMedium = '{',
	CloseMedium = '}',
	OpenBig = '[',
	CloseBig = ']',
	Colon = ':',
	Dot = '.',

	Optional_Nilable = '?',

	Type_Var = 256,
	Type_Let,
	Type_Nil,
	Type_Int,
	Type_Double,
	Type_String,
	Type_Array,
	Type_Dictionary,
	Type_Tuple,
	Type_UserType,

	Stmt_If,
	Stmt_Else,
	Stmt_Guard,
	Stmt_While,
	Stmt_For,
	Stmt_In,

	Stmt_Public,
	Stmt_Private,
	Stmt_Inherit,	//protected

	Func_Decl,
	Func_ReturnType,
	Func_Call,
	Func_Return,

	Struct_Decl,
	Class_Decl,
	Protocol_Decl,
	Extension_Decl,

	Operator_Prefix,
	Operator_Infix,
	Operator_Postfix,

	Identifier,	//������ Ŭ������ �Լ��� ����� ���� �̸�

	Literal_Integer, //����
	Literal_Double, //�ε��Ҽ���
	Literal_Letter,	//����
	Literal_String,	//���ڿ�

	Annotation_Line,	//�ּ�
	Annotation_Block_Open,	// /*
	Annotation_Block_Close,	// */
	LessEqual,
	GreatEqual,
	Equal,
	NotEqual,
};

struct Token
{
	TokenID _id;
	int _line;
	double _number;
	std::string _name;

	Token(TokenID id, int line, double number = 0, std::string name = "") :
		_id(id),
		_line(line),
		_number(number),
		_name(name)
	{ }

	Token(TokenID id, int line, std::string name = "") :
		_id(id),
		_line(line),
		_number(0),
		_name(name)
	{ }
};

using TokenList = std::list<Token>;
using TokenListIter = TokenList::iterator;

#endif
