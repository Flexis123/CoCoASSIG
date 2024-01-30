define void @f() {
  %a = alloca i32, i32 10
  %a.off1 = getelementptr i32, i32* %a, i32 1
  %a.off2 = getelementptr i32, i32* %a.off1, i32 4
  %a.off3 = getelementptr i32, i32* %a.off2, i32 3
  %a.off4 = getelementptr i32, i32* %a.off2, i32 4
  store i32 1337, i32* %a.off2
  ret void
}