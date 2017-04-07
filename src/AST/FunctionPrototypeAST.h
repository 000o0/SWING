#ifndef _SWING_FUNC_PROTO_AST_H_
#define _SWING_FUNC_PROTO_AST_H_

#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include "SwingCompiler.h"
#include "StmtAST.h"

namespace swing
{
	class FunctionProtoAST : StmtAST
	{
		std::string _funcName;
		std::vector<std::string> _args;
	public:
		llvm::Function* CodeGen() override
		{
			std::vector<llvm::Type *> arg;
			llvm::FunctionType *FT =
				llvm::FunctionType::get(llvm::StructType::get(llvm::Type::getInt8Ty(g_SwingCompiler.GetLLVMContext())), false);

			llvm::Function *F =
				llvm::Function::Create(FT, llvm::Function::ExternalLinkage, _funcName, &g_SwingCompiler.GetModule());

			/*
			 *std::unordered_map<std::string, Type> _userTypes;

			_userTypes[TypeName] = Type

				class Type {
				llvm::Type* _type;
			};

			class StructType : Type {
				_type = llvm::StructType;

				std::unordered_map<std::string, std::functional<llvm::Value* (std::unordered_map<std::string, Type>)>>
			};
			*/

			// �� �������� ��� �ϳ��� �����̸� �Լ��� �ϳ��� ����. Name mangling�� �������.
			// �ݺ��ڰ� ����ִ� �ݺ����� �ظ��ϸ� �� �������for�� �ٲ���.
			unsigned Idx = 0;
			for (auto &Arg : F->args())
				Arg.setName(_args[Idx++]);

			return F;
		}
	};
}

#endif
