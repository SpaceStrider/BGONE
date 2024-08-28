AudioSignalResClass {
 Inputs {
  IOPItemInputClass {
   id 1
   name "LockingState"
   tl -200 0
   children {
    4
   }
  }
  IOPInputValueClass {
   id 5
   name "Value 5"
   tl -191 86
   children {
    4
   }
   value 100
  }
 }
 Ops {
  IOPItemOpConditionClass {
   id 4
   name "Cond 4"
   tl -10 3
   children {
    2
   }
   inputs {
    ConnectionClass connection {
     id 5
     port 1
    }
    ConnectionClass connection {
     id 1
     port 0
    }
   }
   "Condition Type" ">="
  }
 }
 Outputs {
  IOPItemOutputClass {
   id 2
   name "LockingState"
   tl 200 0
   input 4
  }
 }
 compiled IOPCompiledClass {
  visited {
   133 5 7 6
  }
  ins {
   IOPCompiledIn {
    data {
     1 3
    }
   }
   IOPCompiledIn {
    data {
     1 3
    }
   }
  }
  ops {
   IOPCompiledOp {
    data {
     1 2 4 65536 1 0 0
    }
   }
  }
  outs {
   IOPCompiledOut {
    data {
     0
    }
   }
  }
  processed 4
  version 2
 }
}