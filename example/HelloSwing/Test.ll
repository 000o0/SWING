; ModuleID = 'SwingCompiler'
source_filename = "main.swing"

@0 = private unnamed_addr constant [13 x i8] c"Hello Swing!\00"

declare void @output(i8*)

declare void @outputInt(i32)

declare void @outputFloat(float)

declare void @outputDouble(double)

define i32 @main() {
entry:
  call void @output(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @0, i32 0, i32 0))
  ret i32 0
}
