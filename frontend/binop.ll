; ModuleID = '<string>'
source_filename = "<string>"
target triple = "x86_64-pc-linux-gnu"

define void @foo() {
entry:
  %i = alloca i32
  %.2 = add i32 1, 2
  store i32 %.2, i32* %i
  %i.1 = load i32, i32* %i
  %.4 = mul i32 %i.1, 1
  store i32 %.4, i32* %i
  %i.2 = load i32, i32* %i
  %.6 = sdiv i32 %i.2, 1
  store i32 %.6, i32* %i
  %i.3 = load i32, i32* %i
  %.8 = srem i32 %i.3, 1
  store i32 %.8, i32* %i
  %f = alloca double
  %.10 = fadd double 1.000000e+00, 2.000000e+00
  store double %.10, double* %f
  %f.1 = load double, double* %f
  %.12 = fadd double %f.1, 4.000000e+00
  store double %.12, double* %f
  %f.2 = load double, double* %f
  %.14 = fdiv double %f.2, 4.000000e+00
  store double %.14, double* %f
  %b = alloca i1
  %i.4 = load i32, i32* %i
  %.16 = icmp slt i32 %i.4, 10
  store i1 %.16, i1* %b
  %i.5 = load i32, i32* %i
  %.18 = icmp sgt i32 %i.5, 10
  store i1 %.18, i1* %b
  %i.6 = load i32, i32* %i
  %.20 = icmp sle i32 %i.6, 10
  store i1 %.20, i1* %b
  %i.7 = load i32, i32* %i
  %.22 = icmp sge i32 %i.7, 10
  store i1 %.22, i1* %b
  %i.8 = load i32, i32* %i
  %.24 = icmp eq i32 %i.8, 10
  store i1 %.24, i1* %b
  %i.9 = load i32, i32* %i
  %.26 = icmp ne i32 %i.9, 10
  store i1 %.26, i1* %b
  %f.3 = load double, double* %f
  %.28 = fcmp oeq double %f.3, 1.100000e+01
  store i1 %.28, i1* %b
  %f.4 = load double, double* %f
  %.30 = fcmp une double %f.4, 1.100000e+01
  store i1 %.30, i1* %b
  %f.5 = load double, double* %f
  %.32 = fcmp olt double %f.5, 1.100000e+01
  store i1 %.32, i1* %b
  %f.6 = load double, double* %f
  %.34 = fcmp ogt double %f.6, 1.100000e+01
  store i1 %.34, i1* %b
  %f.7 = load double, double* %f
  %.36 = fcmp ole double %f.7, 1.100000e+01
  store i1 %.36, i1* %b
  %f.8 = load double, double* %f
  %.38 = fcmp oge double %f.8, 1.100000e+01
  store i1 %.38, i1* %b
  %b.1 = load i1, i1* %b
  br i1 %b.1, label %yes, label %no

yes:                                              ; preds = %entry
  br label %endcond

no:                                               ; preds = %entry
  %b.2 = load i1, i1* %b
  br label %endcond

endcond:                                          ; preds = %no, %yes
  %.41 = phi i1 [ true, %yes ], [ %b.2, %no ]
  store i1 %.41, i1* %b
  %b.3 = load i1, i1* %b
  br i1 %b.3, label %yes.1, label %no.1

yes.1:                                            ; preds = %endcond
  %b.4 = load i1, i1* %b
  br label %endcond.1

no.1:                                             ; preds = %endcond
  br label %endcond.1

endcond.1:                                        ; preds = %no.1, %yes.1
  %.46 = phi i1 [ %b.4, %yes.1 ], [ false, %no.1 ]
  store i1 %.46, i1* %b
  ret void
}
