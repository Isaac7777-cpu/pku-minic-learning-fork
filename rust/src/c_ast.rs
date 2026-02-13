use koopa::ir::Type;

#[derive(Debug)]
pub struct CompUnit {
    pub func_def: FuncDef,
}

#[derive(Debug)]
pub struct FuncDef {
    pub func_type: FuncType,
    pub ident: String,
    pub block: Block,
}

#[derive(Debug)]
pub enum FuncType {
    Int,
}

impl FuncType {
    pub fn to_koopa_type(&self) -> Type {
        match self {
            Self::Int => Type::get_i32(),
        }
    }
}

#[derive(Debug)]
pub struct Block {
    pub stmt: Stmt,
}

#[derive(Debug)]
pub struct Stmt {
    pub exp: Exp,
}

#[derive(Debug)]
pub struct Exp {
    pub add_exp: Box<AddExp>,
}

#[derive(Debug)]
pub enum PrimaryExp {
    Exp { exp: Box<Exp> },
    Number { num: Number },
}

pub type Number = i32;

#[derive(Debug)]
pub enum UnaryExp {
    Primary {
        primary_exp: Box<PrimaryExp>,
    },
    OpUnary {
        unary_op: UnaryOp,
        unary_exp: Box<UnaryExp>,
    },
}

#[derive(Debug)]
pub enum UnaryOp {
    PLUS,
    MINUS,
    BANG,
    TILDE,
}

#[derive(Debug)]
pub enum MulExp {
    Unary {
        unary_exp: Box<UnaryExp>,
    },
    MulOpUnary {
        mul_exp: Box<MulExp>,
        op: MulOp,
        unary_exp: Box<UnaryExp>,
    },
}

#[derive(Debug)]
pub enum AddExp {
    Mul {
        mul_exp: Box<MulExp>,
    },
    AddOpMul {
        add_exp: Box<AddExp>,
        op: AddOp,
        mul_exp: Box<MulExp>,
    },
}

#[derive(Debug)]
pub enum MulOp {
    STAR,
    SLASH,
    PERCENT,
}

#[derive(Debug)]
pub enum AddOp {
    PLUS,
    MINUS,
}
