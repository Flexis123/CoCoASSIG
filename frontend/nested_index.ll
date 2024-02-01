; ModuleID = '<string>'
source_filename = "<string>"
target triple = "x86_64-pc-linux-gnu"

define i32 @main() {
entry:
  %arr = alloca i32, i32 10
  %.2 = bitcast i32* %arr to i8*
  %nbytes = mul i32 10, 4
  call void @llvm.memset.p0i8.i32(i8* %.2, i8 0, i32 %nbytes, i1 false)
  %.4 = sub i32 0, 1
  %arr.idx = getelementptr i32, i32* %arr, i32 %.4
  %.5 = mul i32 3, 2
  store i32 %.5, i32* %arr.idx
  ret i32 0
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.memset.p0i8.i32(i8* nocapture writeonly, i8, i32, i1 immarg) #0

attributes #0 = { argmemonly nounwind willreturn }
