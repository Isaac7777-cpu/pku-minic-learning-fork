use std::fmt::Display;

use crate::register::Reg;

pub enum AsmInst {
    Li { rd: Reg, imm: i32 },
    Mv { rd: Reg, rs: Reg },
    Add { rd: Reg, rs1: Reg, rs2: Reg },
    Sub { rd: Reg, rs1: Reg, rs2: Reg },
    Mul { rd: Reg, rs1: Reg, rs2: Reg },
    Div { rd: Reg, rs1: Reg, rs2: Reg },
    Mod { rd: Reg, rs1: Reg, rs2: Reg },
    Xor { rd: Reg, rs1: Reg, rs2: Reg },
    Seqz { rd: Reg, rs: Reg },
    Ret,
}

impl Display for AsmInst {
    #[rustfmt::skip]
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            // Read instructions
            AsmInst::Li { rd, imm }       => write!(f, "\tli    {rd}, {imm}"),
            AsmInst::Mv { rd, rs }        => write!(f, "\tmv    {rd}, {rs}"),
            AsmInst::Seqz { rd, rs }      => write!(f, "\tseqz  {rd}, {rs}"),
            // Control flow instructions
            AsmInst::Ret                  => write!(f, "\tret"),
            // Arithmetic Instructions
            AsmInst::Add { rd, rs1, rs2 } => write!(f, "\tadd   {rd}, {rs1}, {rs2}"),
            AsmInst::Sub { rd, rs1, rs2 } => write!(f, "\tsub   {rd}, {rs1}, {rs2}"),
            AsmInst::Mul { rd, rs1, rs2 } => write!(f, "\tmul   {rd}, {rs1}, {rs2}"),
            AsmInst::Div { rd, rs1, rs2 } => write!(f, "\tdiv   {rd}, {rs1}, {rs2}"),
            AsmInst::Mod { rd, rs1, rs2 } => write!(f, "\trem   {rd}, {rs1}, {rs2}"),
            AsmInst::Xor { rd, rs1, rs2 } => write!(f, "\txor   {rd}, {rs1}, {rs2}"),
        }
    }
}
