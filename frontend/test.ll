; ModuleID = '<string>'
source_filename = "<string>"
target triple = "x86_64-pc-linux-gnu"

define i32 @main() {
entry:
  %i = alloca i32
  store i32 0, i32* %i
  %i.1 = alloca i32
  store i32 0, i32* %i.1
  %_end = alloca i32
  store i32 20, i32* %_end
  br label %entry.whilecond

entry.whilecond:                                  ; preds = %entry.whilebody, %entry
  %i.2 = load i32, i32* %i.1
  %_end.1 = load i32, i32* %_end
  %.6 = icmp slt i32 %i.2, %_end.1
  br i1 %.6, label %entry.whilebody, label %entry.whileend

entry.whilebody:                                  ; preds = %entry.whilecond
  %i.3 = alloca i32
  store i32 0, i32* %i.3
  %i.4 = load i32, i32* %i.1
  %.9 = add i32 %i.4, 1
  store i32 %.9, i32* %i.1
  br label %entry.whilecond

entry.whileend:                                   ; preds = %entry.whilecond
  ret i32 0
}
