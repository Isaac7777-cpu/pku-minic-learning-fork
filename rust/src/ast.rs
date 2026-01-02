pub struct CompUnit {
    pub func_def: FuncDef,
}

pub struct FuncDef {
    pub func_type: FuncType,
    pub ident: String,
    pub block: Block,
}

pub struct FuncType {}

pub struct Block {
    pub stmt: Stmt,
}

pub struct Stmt {
    pub number: Number,
}

pub struct Number {
    pub int_const: i32,
}
