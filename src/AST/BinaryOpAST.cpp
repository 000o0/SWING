﻿#include "BinaryOpAST.h"
#include "UnaryOpAST.h"
#include "VariableAST.h"
#include "Type.h"
#include "Error.h"

namespace swing
{
	ExprAST::ExprPtr BinaryOpAST::Create(TokenIter& iter, int PrecedenceLevel)
	{
		auto ast = std::shared_ptr<BinaryOpAST>();
		ast.reset(new BinaryOpAST());

		/// 연산자 우선순위가 높은 항 부터 찾아서 파싱해야 한다.
		do
		{
			ast->_opCandidates = g_SwingCompiler->FindOps(++PrecedenceLevel);
		}
		while (PrecedenceLevel < OperatorType::Precedence_Max && ast->_opCandidates.size() == 0);

		/// LHS항 넣기.
		if (PrecedenceLevel >= OperatorType::Precedence_Max)
			ast->_exprList.push_back(UnaryOpAST::Create(iter));
		else
			ast->_exprList.push_back(BinaryOpAST::Create(iter, PrecedenceLevel));

		while (true)
		{
			/// operator 찾기.
			int size = ast->_opTypes.size();
			for (auto op : ast->_opCandidates)
			{
				if (iter->_name == op->_opString)
				{
					ast->_opTypes.push_back(op);
					break;
				}
			}

			if (size == ast->_opTypes.size())
				break;

			++iter;
			/// RHS항 넣기.
			ast->_exprList.push_back(BinaryOpAST::Create(iter, PrecedenceLevel));
		}
		
		return ast->_opTypes.size() == 0 ? ast->_exprList.front() : ExprPtr(ast);
	}

	BinaryOpAST::~BinaryOpAST()
	{
	}

	llvm::Type* BinaryOpAST::GetType()
	{
		return _exprList.begin()->get()->GetType();
	}

	llvm::Value* BinaryOpAST::CodeGen()
	{
		/// Built-in 한 op들은 if else를 통해 문자열 비교해서 codegen한다.
		/// 아닌것들은 opmangling된 이름을 가지고 function call 한다.

		/// _exprList의 항이 하나만 있으면 걍 해당식 CodeGen을 리턴한다. 부가연산 없다.

		llvm::Value* value = nullptr;
		
		auto initOp = (*_opTypes.begin())->_tokenID;
		if (initOp != TokenID::Assignment && initOp != TokenID::MemberReference)	
			value = _exprList.begin()->get()->CodeGen();

		llvm::Type* type;
		ExprList::iterator rhs = next(_exprList.begin());

		for (auto opIter = _opTypes.begin(); opIter != _opTypes.end(); ++opIter)
		{
			switch ((*opIter)->_tokenID)
			{
			case TokenID::Assignment:
			{
				if (_opTypes.size() > 1)
					throw Error("Only one Assignment per line is allowed.");

				//VariableExprAST* varAST = dynamic_cast<VariableExprAST*>(_exprList[0].get());

				return g_Builder.CreateStore(rhs->get()->CodeGen(), _exprList.front().get()->CodeGenRef());
			}
			case TokenID::MemberReference:
			{
				if (opIter == _opTypes.begin())
					value = _exprList.front().get()->CodeGenRef();

				VariableExprAST* rhsMember = nullptr;
				rhsMember = dynamic_cast<VariableExprAST*>(rhs->get());
				if (rhsMember != nullptr)
				{
					/// GetElementPtr을 여기로 들고오고, 인덱스 들고오는 것만 StructType에서 처리
					int idx = g_SwingCompiler->_structs[value->getType()->getPointerElementType()->getStructName()]->GetElement(rhsMember->_variableName)->_idx;
					value = g_Builder.CreateStructGEP(value->getType()->getPointerElementType(), value, idx);

					if (next(opIter) == _opTypes.end())
					{
						return g_Builder.CreateLoad(value);
					}
				}
				else
				{
					/// Function Call 일 것 이다.
					FunctionCallAST* rhsFunction = nullptr;
					rhsFunction = dynamic_cast<FunctionCallAST*>(rhs->get());
					std::string structName = value->getType()->getPointerElementType()->getStructName();
					std::string funcName = structName + "." + rhsFunction->_funcName;
					rhsFunction->_funcName = funcName;
					Method* method = g_SwingCompiler->GetFunction(funcName);

					if (!method->_func)
					{
						/// Initialize Self arg.
						rhsFunction->_callArgs.push_back(value);
						rhsFunction->_args["self"] = nullptr;
						method->_args.insert(method->_args.begin(), new Variable(value->getType(), "self", false, false, false));

						method->DeclareMethod(g_SwingCompiler->_structs[structName]);
						g_SwingCompiler->ImplementFunctionLazy(method);
					}
					
					value = rhsFunction->CodeGen();
				}
			}
			break;
			case TokenID::Arithmetic_Add:
				type = value->getType();

				if (type == Char || type == Int)
				{
					value = g_Builder.CreateAdd(value, rhs++->get()->CodeGen());
				}
				else if (type == Float || type == Double)
				{
					value = g_Builder.CreateFAdd(value, rhs++->get()->CodeGen());
				}
				else
				{
					/// TODO : User Defined Function Call.
				}
				break;
			case TokenID::Arithmetic_Subtract:
				type = value->getType();

				if (type == Char || type == Int)
				{
					value = g_Builder.CreateSub(value, rhs++->get()->CodeGen());
				}
				else if (type == Float || type == Double)
				{
					value = g_Builder.CreateFSub(value, rhs++->get()->CodeGen());
				}
				else
				{
					/// TODO : User Defined Function Call.
				}
				break;
			case TokenID::Arithmetic_Multiply:
				type = value->getType();

				if (type == Char || type == Int)
				{
					value = g_Builder.CreateMul(value, rhs++->get()->CodeGen());
				}
				else if (type == Float || type == Double)
				{
					value = g_Builder.CreateFMul(value, rhs++->get()->CodeGen());
				}
				else
				{
					/// TODO : User Defined Function Call.
				}
				break;
			case TokenID::Arithmetic_Divide:
				type = value->getType();

				if (type == Char || type == Int)
				{
					llvm::Value* rhsValue = rhs++->get()->CodeGen();
					if (rhsValue == llvm::ConstantInt::get(Char, llvm::APInt(8, 0)))
						throw Error("Expression is divided by zero!");
					if (rhsValue == llvm::ConstantInt::get(Int, llvm::APInt(32, 0)))
						throw Error("Expression is divided by zero!");
					value = g_Builder.CreateSDiv(value, rhsValue);
				}
				else if (type == Float || type == Double)
				{
					llvm::Value* rhsValue = rhs++->get()->CodeGen();
					if (rhsValue == llvm::ConstantFP::get(Float, 0.0f))
						throw Error("Expression is divided by zero!");
					if (rhsValue == llvm::ConstantFP::get(Double, 0.0f))
						throw Error("Expression is divided by zero!");
					value = g_Builder.CreateFDiv(value, rhsValue);
				}
				else
				{
					/// TODO : User Defined Function Call.
				}
				break;
			case TokenID::Arithmetic_Modulo:
				type = value->getType();

				if (type == Char || type == Int)
				{
					value = g_Builder.CreateSRem(value, rhs++->get()->CodeGen());
				}
				else if (type == Float || type == Double)
				{
					value = g_Builder.CreateFRem(value, rhs++->get()->CodeGen());
				}
				else
				{
					/// TODO : User Defined Function Call.
				}
				break;
			//case TokenID::Arithmetic_Power:
			//	break;
			case TokenID::Logical_And:
				type = value->getType();

				if (type == Bool)
				{
					value = g_Builder.CreateAnd(value, rhs++->get()->CodeGen());
				}
				else
				{
					/// TODO : User Defined Function Call.
					throw Error("And expr type is not bool type!");
				}
				break;
			case TokenID::Logical_Or:
				type = value->getType();

				if (type == Bool)
				{
					value = g_Builder.CreateOr(value, rhs++->get()->CodeGen());
				}
				else
				{
					/// TODO : User Defined Function Call.
					throw Error("Or expr type is not bool type!");
				}
				break;
			case TokenID::Relational_Equal:
				type = value->getType();

				if (type == Bool || type == Char || type == Int)
				{
					value = g_Builder.CreateICmpEQ(value, rhs++->get()->CodeGen());
				}
				else if (type == Float || type == Double)
				{
					value = g_Builder.CreateFCmpOEQ(value, rhs++->get()->CodeGen());
				}
				else
				{
					/// TODO : User Defined Function Call.
				}
				break;
			case TokenID::Relational_NotEqual:
				type = value->getType();

				if (type == Bool || type == Char || type == Int)
				{
					value = g_Builder.CreateICmpNE(value, rhs++->get()->CodeGen());
				}
				else if (type == Float || type == Double)
				{
					value = g_Builder.CreateFCmpONE(value, rhs++->get()->CodeGen());
				}
				else
				{
					/// TODO : User Defined Function Call.
				}
				break;
			case TokenID::Relational_Greater:
				type = value->getType();

				if (type == Bool || type == Char || type == Int)
				{
					value = g_Builder.CreateICmpSGT(value, rhs++->get()->CodeGen());
				}
				else if (type == Float || type == Double)
				{
					value = g_Builder.CreateFCmpOGT(value, rhs++->get()->CodeGen());
				}
				else
				{
					/// TODO : User Defined Function Call.
				}
				break;
			case TokenID::Relational_GreaterEqual:
				type = value->getType();

				if (type == Bool || type == Char || type == Int)
				{
					value = g_Builder.CreateICmpSGE(value, rhs++->get()->CodeGen());
				}
				else if (type == Float || type == Double)
				{
					value = g_Builder.CreateFCmpOGE(value, rhs++->get()->CodeGen());
				}
				else
				{
					/// TODO : User Defined Function Call.
				}
				break;
			case TokenID::Relational_Less:
				type = value->getType();

				if (type == Bool || type == Char || type == Int)
				{
					value = g_Builder.CreateICmpSLT(value, rhs++->get()->CodeGen());
				}
				else if (type == Float || type == Double)
				{
					value = g_Builder.CreateFCmpOLT(value, rhs++->get()->CodeGen());
				}
				else
				{
					/// TODO : User Defined Function Call.
				}
				break;
			case TokenID::Relational_LessEqual:
				type = value->getType();

				if (type == Bool || type == Char || type == Int)
				{
					value = g_Builder.CreateICmpSLE(value, rhs++->get()->CodeGen());
				}
				else if (type == Float || type == Double)
				{
					value = g_Builder.CreateFCmpOLE(value, rhs++->get()->CodeGen());
				}
				else
				{
					/// TODO : User Defined Function Call.
				}
				break;

			default:
				throw Error("Unknown infix Operator");
			}
		}

		return value;
	}

	llvm::Value* BinaryOpAST::CodeGenRef()
	{
		/// TODO : Member Reference Only.

		llvm::Value* value = _exprList.front().get()->CodeGenRef();
		auto initOp = (*_opTypes.begin())->_tokenID;
		ExprList::iterator rhs = next(_exprList.begin());

		for (auto opIter = _opTypes.begin(); opIter != _opTypes.end(); ++opIter)
		{
			if ((*opIter)->_tokenID == TokenID::MemberReference)
			{
				/// 정말 마음에 안들지만 확실하고 빠른 방법이다.
				
				VariableExprAST* rhsMember = nullptr;
				rhsMember = dynamic_cast<VariableExprAST*>(rhs->get());
				if (rhsMember != nullptr)
				{
					value = g_SwingCompiler->_structs[value->getType()->getPointerElementType()->getStructName()]->GetElementPtr(value, rhsMember->_variableName);
				}
				else
				{
					/// Function Call 일 것 이다.
					FunctionCallAST* rhsFunction = nullptr;
					rhsFunction = dynamic_cast<FunctionCallAST*>(rhs->get());
					std::string funcName = (value->getType()->getPointerElementType()->getStructName() + "." + rhsFunction->_funcName).str();
					rhsFunction->_funcName = funcName;
					value = rhsFunction->CodeGen();
				}
			}
			else
			{
				throw Error("BinaryOpAST Error, Not MemberReference Op.");
			}
		}

		return value;
	}
}
