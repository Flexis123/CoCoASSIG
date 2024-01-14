; ModuleID = '<string>'
source_filename = "<string>"
target triple = "x86_64-pc-linux-gnu"

define void @foo() {
entry:
  br i1 true, label %entry.if, label %entry.endif

entry.if:                                         ; preds = %entry
  ret void

post_return:                                      ; No predecessors!
  br label %entry.endif

entry.endif:                                      ; preds = %post_return, %entry
  br i1 true, label %entry.endif.if, label %entry.endif.else

entry.endif.if:                                   ; preds = %entry.endif
  ret void

post_return.1:                                    ; No predecessors!
  br label %entry.endif.endif

entry.endif.else:                                 ; preds = %entry.endif
  ret void

post_return.2:                                    ; No predecessors!
  br label %entry.endif.endif

entry.endif.endif:                                ; preds = %post_return.2, %post_return.1
  ret void
}

define i32 @bar() {
entry:
  br i1 true, label %entry.if, label %entry.endif

entry.if:                                         ; preds = %entry
  br i1 false, label %entry.if.if, label %entry.if.else

entry.if.if:                                      ; preds = %entry.if
  ret i32 0

post_return:                                      ; No predecessors!
  br label %entry.if.endif

entry.if.else:                                    ; preds = %entry.if
  ret i32 1

post_return.1:                                    ; No predecessors!
  br label %entry.if.endif

entry.if.endif:                                   ; preds = %post_return.1, %post_return
  br label %entry.endif

entry.endif:                                      ; preds = %entry.if.endif, %entry
  ret i32 2
}

define i32 @baz() {
entry:
  br i1 true, label %entry.if, label %entry.else

entry.if:                                         ; preds = %entry
  br i1 false, label %entry.if.if, label %entry.if.endif

entry.if.if:                                      ; preds = %entry.if
  ret i32 0

post_return:                                      ; No predecessors!
  br label %entry.if.endif

entry.if.endif:                                   ; preds = %post_return, %entry.if
  br label %entry.endif

entry.else:                                       ; preds = %entry
  ret i32 1

post_return.1:                                    ; No predecessors!
  br label %entry.endif

entry.endif:                                      ; preds = %post_return.1, %entry.if.endif
  ret i32 2
}
