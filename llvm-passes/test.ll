
define i32 @main(i32 %param.argc, i8** %argv) {
  %a = alloca i32, i32 16
  %a.idx = getelementptr i32, i32* %a, i32 100
  store i32 1337, i32* %a.idx

  %b = getelementptr i8*, i8** %argv, i32 2
  ret i32 0
}