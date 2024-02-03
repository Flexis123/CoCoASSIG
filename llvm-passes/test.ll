; ModuleID = '<string>'
source_filename = "<string>"
target triple = "x86_64-pc-linux-gnu"

define i32 @main(i32 %param.argc, i8** %argv) {
entry:
  %argc = alloca i32
  store i32 %param.argc, i32* %argc
  %a = alloca i32, i32 100
  %.5 = bitcast i32* %a to i8*
  %nbytes = mul i32 100, 4
  call void @llvm.memset.p0i8.i32(i8* %.5, i8 0, i32 %nbytes, i1 false)
  %argc.1 = load i32, i32* %argc
  %a.ptr = getelementptr i32, i32* %a, i32 %argc.1
  %a.idx = load i32, i32* %a.ptr
  ret i32 %a.idx
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.memset.p0i8.i32(i8* nocapture writeonly, i8, i32, i1 immarg) #0

attributes #0 = { argmemonly nounwind willreturn }
