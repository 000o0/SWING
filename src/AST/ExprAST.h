﻿#ifndef _SWING_EXPR_AST_H_
#define _SWING_EXPR_AST_H_

#include "BaseAST.h"
#include "SwingCompiler.h"

namespace swing
{
	/*enum class ExprType
	{
		TopExpr,
		BinaryOp,
		UnaryOp,
		Variable,
		FunctionCall
	};*/

	class ExprAST : public BaseAST
	{
	public:
		virtual ~ExprAST() {}

		using ExprPtr = std::shared_ptr<ExprAST>;

		static ExprPtr CreateTopLevelExpr(TokenIter& iter);
		virtual llvm::Type* GetType() { return nullptr; }
		//virtual ExprType GetExprType() { return ExprType::TopExpr; }
		virtual llvm::Value* CodeGenRef() { return nullptr; }
		llvm::Value* CodeGen() override;
	};

	using ExprList = std::vector<ExprAST::ExprPtr>;
	using ExprIter = ExprList::iterator;


	class ExprListAST : public BaseAST
	{
	public:
		/// TODO : 기능 구현.. expr1, expr2, expr3 이런거 담고있는 데이터.
		virtual ~ExprListAST() {}

		ExprList _exprList;

		using ExprPtr = std::shared_ptr<ExprAST>;
		static ExprPtr Create(TokenIter& iter);
		llvm::Value* CodeGen() override;
	};
}

#endif
