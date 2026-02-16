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
    And { rd: Reg, rs1: Reg, rs2: Reg },
    Or { rd: Reg, rs1: Reg, rs2: Reg },
    Xor { rd: Reg, rs1: Reg, rs2: Reg },
    Xori { rd: Reg, rs: Reg, imm: i16 },
    Slt { rd: Reg, rs1: Reg, rs2: Reg },
    Sgt { rd: Reg, rs1: Reg, rs2: Reg },
    Seqz { rd: Reg, rs: Reg },
    Snez { rd: Reg, rs: Reg },
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
            AsmInst::Snez { rd, rs }      => write!(f, "\tsnez  {rd}, {rs}"),
            // Control flow instructions
            AsmInst::Ret                  => write!(f, "\tret"),
            // Arithmetic Instructions
            AsmInst::Add { rd, rs1, rs2 } => write!(f, "\tadd   {rd}, {rs1}, {rs2}"),
            AsmInst::Sub { rd, rs1, rs2 } => write!(f, "\tsub   {rd}, {rs1}, {rs2}"),
            AsmInst::Mul { rd, rs1, rs2 } => write!(f, "\tmul   {rd}, {rs1}, {rs2}"),
            AsmInst::Div { rd, rs1, rs2 } => write!(f, "\tdiv   {rd}, {rs1}, {rs2}"),
            AsmInst::Mod { rd, rs1, rs2 } => write!(f, "\trem   {rd}, {rs1}, {rs2}"),
            AsmInst::And { rd, rs1, rs2 } => write!(f, "\tand   {rd}, {rs1}, {rs2}"),
            AsmInst::Or { rd, rs1, rs2 }  => write!(f, "\tor    {rd}, {rs1}, {rs2}"),
            AsmInst::Xor { rd, rs1, rs2 } => write!(f, "\txor   {rd}, {rs1}, {rs2}"),
            AsmInst::Xori { rd, rs, imm } => write!(f, "\txori  {rd}, {rs}, {imm}"),
            AsmInst::Slt { rd, rs1, rs2 } => write!(f, "\tslt   {rd}, {rs1}, {rs2}"),
            AsmInst::Sgt { rd, rs1, rs2 } => write!(f, "\tslt   {rd}, {rs2}, {rs1}"),
        }
    }
}
