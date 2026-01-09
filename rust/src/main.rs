use koopa::back::KoopaGenerator;
use lalrpop_util::lalrpop_mod;
use std::env::args;
use std::fs::{File, read_to_string};
use std::io::{BufWriter, Result, Write};

use crate::koopa_ast::LowerCtx;

mod c_ast;
mod koopa_ast;

// Follow the name of the just created xxxx.lalrpop
lalrpop_mod!(sysy);

fn main() -> Result<()> {
    let mut args = args();
    args.next();
    let mode = args.next().unwrap();
    let input = args.next().unwrap();
    args.next();
    let output = args.next().unwrap();

    let input = read_to_string(input)?;

    let ast = sysy::CompUnitParser::new().parse(&input).unwrap();

    println!("{:#?}", ast);

    let mut lower_ctx = LowerCtx::new();
    lower_ctx.build_program_from_comp_unit(ast);

    let file = File::create(output.as_str())
        .expect(format!("Unable to write to output file: {}", output).as_str());
    let mut writer = BufWriter::new(file);

    let mut generator = KoopaGenerator::new(&mut writer);
    generator
        .generate_on(&lower_ctx.program)
        .expect("Unable to generate koopa IR from custom koopa representation.");

    writer
        .flush()
        .expect(format!("Unable to write to output file: {}", output).as_str());

    Ok(())
}
