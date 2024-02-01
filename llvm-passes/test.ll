define void @print10th(i32* %arr) {
entry:
  %.str.0 = getelementptr inbounds [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %arr.ptr = getelementptr i32, i32* %arr, i32 10
  %arr.idx = load i32, i32* %arr.ptr
  %.3 = call i32 (i8*, ...) @printf(i8* %.str.0, i32 %arr.idx)
  ret void
}
define i32 @main(i32 %param.argc, i8** %argv) {
entry:
  %a = alloca i32, i32 8
  %b = alloca i32, i32 16

  call void @print10th(i32* %a)
  call void @print10th(i32* %b)
  ret i32 0
}