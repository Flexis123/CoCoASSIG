; ModuleID = '<string>'
source_filename = "<string>"
target triple = "x86_64-pc-linux-gnu"

@.str.0 = unnamed_addr constant [4 x i8] c"foo\00"
@.str.1 = unnamed_addr constant [4 x i8] c"YES\00"
@.str.2 = unnamed_addr constant [4 x i8] c"foo\00"
@.str.3 = unnamed_addr constant [3 x i8] c"NO\00"

declare i32 @random()

declare void @srandom(i32)

declare i32 @printf(i8*, ...)

declare i32 @puts(i8*)

declare i32 @atoi(i8*)

declare void @exit(i32)

define i32 @main(i32 %param.argc, i8** %argv) {
entry:
  %argc = alloca i32
  store i32 %param.argc, i32* %argc
  %i = alloca i32
  store i32 0, i32* %i
  %_end = alloca i32
  store i32 4, i32* %_end
  br label %entry.whilecond

entry.whilecond:                                  ; preds = %entry.whilebody, %entry
  %i.1 = load i32, i32* %i
  %_end.1 = load i32, i32* %_end
  %.8 = icmp slt i32 %i.1, %_end.1
  br i1 %.8, label %entry.whilebody, label %entry.whileend

entry.whilebody:                                  ; preds = %entry.whilecond
  %a = alloca i8
  %.str.0 = getelementptr inbounds [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %i.2 = load i32, i32* %i
  %.10 = getelementptr i8, i8* %.str.0, i32 %i.2
  %.11 = load i8, i8* %.10
  store i8 %.11, i8* %a
  %i.3 = load i32, i32* %i
  %.13 = add i32 %i.3, 1
  store i32 %.13, i32* %i
  br label %entry.whilecond

entry.whileend:                                   ; preds = %entry.whilecond
  %.str.1 = getelementptr inbounds [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %.16 = call i32 @puts(i8* %.str.1)
  %b = alloca i8
  %.str.2 = getelementptr inbounds [4 x i8], [4 x i8]* @.str.2, i32 0, i32 0
  %.17 = getelementptr i8, i8* %.str.2, i32 4
  %.18 = load i8, i8* %.17
  store i8 %.18, i8* %b
  %.str.3 = getelementptr inbounds [3 x i8], [3 x i8]* @.str.3, i32 0, i32 0
  %.20 = call i32 @puts(i8* %.str.3)
  ret i32 0
}
