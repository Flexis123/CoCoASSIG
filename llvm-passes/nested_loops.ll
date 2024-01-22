; ModuleID = '<string>'
source_filename = "<string>"
target triple = "x86_64-pc-linux-gnu"

define i32 @main() {
entry:
  %pula = alloca i32
  store i32 0, i32* %pula
  %_end5 = alloca i32
  store i32 20, i32* %_end5
  %i = alloca i32
  store i32 0, i32* %i
  br label %entry.whilecond

entry.whilecond:                                  ; preds = %entry.whilebody.whileend.whileend, %entry
  %i.1 = load i32, i32* %i
  %_end5.1 = load i32, i32* %_end5
  %.6 = icmp slt i32 %i.1, %_end5.1
  br i1 %.6, label %entry.whilebody, label %entry.whileend

entry.whilebody:                                  ; preds = %entry.whilecond
  %_end3 = alloca i32
  store i32 7, i32* %_end3
  %x = alloca i32
  store i32 0, i32* %x
  br label %entry.whilebody.whilecond

entry.whilebody.whilecond:                        ; preds = %entry.whilebody.whilebody.whileend.whileend, %entry.whilebody
  %x.1 = load i32, i32* %x
  %_end3.1 = load i32, i32* %_end3
  %.11 = icmp slt i32 %x.1, %_end3.1
  br i1 %.11, label %entry.whilebody.whilebody, label %entry.whilebody.whileend

entry.whilebody.whilebody:                        ; preds = %entry.whilebody.whilecond
  %_end = alloca i32
  store i32 7, i32* %_end
  %f = alloca i32
  store i32 0, i32* %f
  br label %entry.whilebody.whilebody.whilecond

entry.whilebody.whilebody.whilecond:              ; preds = %entry.whilebody.whilebody.whilebody, %entry.whilebody.whilebody
  %f.1 = load i32, i32* %f
  %_end.1 = load i32, i32* %_end
  %.16 = icmp slt i32 %f.1, %_end.1
  br i1 %.16, label %entry.whilebody.whilebody.whilebody, label %entry.whilebody.whilebody.whileend

entry.whilebody.whilebody.whilebody:              ; preds = %entry.whilebody.whilebody.whilecond
  %pula.1 = load i32, i32* %pula
  %.18 = add i32 %pula.1, 1
  store i32 %.18, i32* %pula
  %f.2 = load i32, i32* %f
  %.20 = add i32 %f.2, 1
  store i32 %.20, i32* %f
  br label %entry.whilebody.whilebody.whilecond

entry.whilebody.whilebody.whileend:               ; preds = %entry.whilebody.whilebody.whilecond
  %_end2 = alloca i32
  store i32 15, i32* %_end2
  %fn = alloca i32
  store i32 0, i32* %fn
  br label %entry.whilebody.whilebody.whileend.whilecond

entry.whilebody.whilebody.whileend.whilecond:     ; preds = %entry.whilebody.whilebody.whileend.whilebody, %entry.whilebody.whilebody.whileend
  %fn.1 = load i32, i32* %fn
  %_end2.1 = load i32, i32* %_end2
  %.26 = icmp slt i32 %fn.1, %_end2.1
  br i1 %.26, label %entry.whilebody.whilebody.whileend.whilebody, label %entry.whilebody.whilebody.whileend.whileend

entry.whilebody.whilebody.whileend.whilebody:     ; preds = %entry.whilebody.whilebody.whileend.whilecond
  %pula.2 = load i32, i32* %pula
  %.28 = add i32 %pula.2, 2
  store i32 %.28, i32* %pula
  %fn.2 = load i32, i32* %fn
  %.30 = add i32 %fn.2, 1
  store i32 %.30, i32* %fn
  br label %entry.whilebody.whilebody.whileend.whilecond

entry.whilebody.whilebody.whileend.whileend:      ; preds = %entry.whilebody.whilebody.whileend.whilecond
  %x.2 = load i32, i32* %x
  %.33 = add i32 %x.2, 1
  store i32 %.33, i32* %x
  br label %entry.whilebody.whilecond

entry.whilebody.whileend:                         ; preds = %entry.whilebody.whilecond
  %_end4 = alloca i32
  store i32 3, i32* %_end4
  %n = alloca i32
  store i32 0, i32* %n
  br label %entry.whilebody.whileend.whilecond

entry.whilebody.whileend.whilecond:               ; preds = %entry.whilebody.whileend.whilebody, %entry.whilebody.whileend
  %n.1 = load i32, i32* %n
  %_end4.1 = load i32, i32* %_end4
  %.39 = icmp slt i32 %n.1, %_end4.1
  br i1 %.39, label %entry.whilebody.whileend.whilebody, label %entry.whilebody.whileend.whileend

entry.whilebody.whileend.whilebody:               ; preds = %entry.whilebody.whileend.whilecond
  %pula.3 = load i32, i32* %pula
  %.41 = sub i32 %pula.3, 1
  store i32 %.41, i32* %pula
  %n.2 = load i32, i32* %n
  %.43 = add i32 %n.2, 1
  store i32 %.43, i32* %n
  br label %entry.whilebody.whileend.whilecond

entry.whilebody.whileend.whileend:                ; preds = %entry.whilebody.whileend.whilecond
  %i.2 = load i32, i32* %i
  %.46 = add i32 %i.2, 1
  store i32 %.46, i32* %i
  br label %entry.whilecond

entry.whileend:                                   ; preds = %entry.whilecond
  ret i32 0
}
