
@X = global i1 0

define i32 @main() #0 {
  entry:
    %0 = load i1, i1* @X
    br label %ret
  unreach:
    %1 = alloca i32
    br label %ret
  ret:
    ret i32 0
}