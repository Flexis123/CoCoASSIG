; ModuleID = '<string>'
source_filename = "<string>"
target triple = "x86_64-pc-linux-gnu"

define i32 @main() {
entry:
  %n = alloca i32
  store i32 0, i32* %n
  %_end2 = alloca i32
  store i32 10, i32* %_end2
  %i = alloca i32
  store i32 0, i32* %i
  br label %entry.whilecond

entry.whilecond:                                  ; preds = %entry.whilebody.whileend, %entry
  %i.1 = load i32, i32* %i
  %_end2.1 = load i32, i32* %_end2
  %.6 = icmp slt i32 %i.1, %_end2.1
  br i1 %.6, label %entry.whilebody, label %entry.whileend

entry.whilebody:                                  ; preds = %entry.whilecond
  store i32 10, i32* %n
  %_end = alloca i32
  store i32 10, i32* %_end
  %f = alloca i32
  store i32 0, i32* %f
  br label %entry.whilebody.whilecond

entry.whilebody.whilecond:                        ; preds = %entry.whilebody.whilebody.endif, %entry.whilebody
  %f.1 = load i32, i32* %f
  %_end.1 = load i32, i32* %_end
  %.12 = icmp slt i32 %f.1, %_end.1
  br i1 %.12, label %entry.whilebody.whilebody, label %entry.whilebody.whileend

entry.whilebody.whilebody:                        ; preds = %entry.whilebody.whilecond
  %f.2 = load i32, i32* %f
  %.14 = icmp eq i32 %f.2, 3
  br i1 %.14, label %entry.whilebody.whilebody.if, label %entry.whilebody.whilebody.endif

entry.whilebody.whilebody.if:                     ; preds = %entry.whilebody.whilebody
  store i32 1, i32* %n
  br label %entry.whilebody.whilebody.endif

entry.whilebody.whilebody.endif:                  ; preds = %entry.whilebody.whilebody.if, %entry.whilebody.whilebody
  store i32 5, i32* %n
  %f.3 = load i32, i32* %f
  %.19 = add i32 %f.3, 1
  store i32 %.19, i32* %f
  br label %entry.whilebody.whilecond

entry.whilebody.whileend:                         ; preds = %entry.whilebody.whilecond
  %i.2 = load i32, i32* %i
  %.22 = add i32 %i.2, 1
  store i32 %.22, i32* %i
  br label %entry.whilecond

entry.whileend:                                   ; preds = %entry.whilecond
  ret i32 0
}
