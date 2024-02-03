; ModuleID = '<string>'
source_filename = "<string>"
target triple = "x86_64-pc-linux-gnu"

@.str.0 = unnamed_addr constant [2 x i8] c"8\00"
@.str.1 = unnamed_addr constant [9 x i8] c"num: %d\0A\00"
@.str.2 = unnamed_addr constant [4 x i8] c"%d\0A\00"

declare i32 @printf(i8*, ...)

declare i32 @atoi(i8*)

define i32 @main(i32 %param.argc, i8** %argv) {
entry:
  %argc = alloca i32
  store i32 %param.argc, i32* %argc
  %num = alloca i32
  %.str.0 = getelementptr inbounds [2 x i8], [2 x i8]* @.str.0, i32 0, i32 0
  %.5 = call i32 @atoi(i8* %.str.0)
  store i32 %.5, i32* %num
  %.str.1 = getelementptr inbounds [9 x i8], [9 x i8]* @.str.1, i32 0, i32 0
  %num.1 = load i32, i32* %num
  %.7 = call i32 (i8*, ...) @printf(i8* %.str.1, i32 %num.1)
  %i = alloca i32
  store i32 0, i32* %i
  %_end = alloca i32
  %num.2 = load i32, i32* %num
  store i32 %num.2, i32* %_end
  br label %entry.whilecond

entry.whilecond:                                  ; preds = %entry.whilebody, %entry
  %i.1 = load i32, i32* %i
  %_end.1 = load i32, i32* %_end
  %.11 = icmp slt i32 %i.1, %_end.1
  br i1 %.11, label %entry.whilebody, label %entry.whileend

entry.whilebody:                                  ; preds = %entry.whilecond
  %.str.2 = getelementptr inbounds [4 x i8], [4 x i8]* @.str.2, i32 0, i32 0
  %num.3 = load i32, i32* %num
  %i.2 = load i32, i32* %i
  %.13 = sub i32 %num.3, %i.2
  %.14 = call i32 (i8*, ...) @printf(i8* %.str.2, i32 %.13)
  %i.3 = load i32, i32* %i
  %.15 = add i32 %i.3, 1
  store i32 %.15, i32* %i
  br label %entry.whilecond

entry.whileend:                                   ; preds = %entry.whilecond
  ret i32 0
}
