use std::fmt::Display;

use crate::register::Reg;

pub enum AsmInst {
    Li { rd: Reg, imm: i32 },
    Mv { rd: Reg, rs: Reg },
    Sub { rd: Reg, rs1: Reg, rs2: Reg },
    Xor { rd: Reg, rs1: Reg, rs2: Reg },
    Seqz { rd: Reg, rs: Reg },
    Ret,
}

impl Display for AsmInst {
    #[rustfmt::skip]
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            AsmInst::Li { rd, imm }       => write!(f, "\tli    {rd}, {imm}"),
            AsmInst::Mv { rd, rs }        => write!(f, "\tmv    {rd}, {rs}"),
            AsmInst::Sub { rd, rs1, rs2 } => write!(f, "\tsub   {rd}, {rs1}, {rs2}"),
            AsmInst::Xor { rd, rs1, rs2 } => write!(f, "\txor   {rd}, {rs1}, {rs2}"),
            AsmInst::Seqz { rd, rs }      => write!(f, "\tseqz  {rd}, {rs}"),
            AsmInst::Ret                  => write!(f, "\tret"),
        }
    }
}
