use crate::register::Reg;

pub trait RegAllocator {
    fn is_avail(&self, reg: Reg) -> bool;
    fn acquire(&self, reg: Reg) -> Option<Reg>;
    fn get_avail(&mut self) -> Option<Reg>;
}

/// For now, this does now handle spilling. It just simply goes from T
pub struct NaiveRegAlloc {
    t_reg: [bool; 7],
    a_reg: [bool; 8],
}

impl RegAllocator for NaiveRegAlloc {
    fn is_avail(&self, reg: Reg) -> bool {
        match reg {
            Reg::T(idx) => !self.t_reg[usize::from(idx)],
            Reg::A(idx) => !self.a_reg[usize::from(idx)],
            _ => false,
        }
    }

    fn acquire(&self, reg: Reg) -> Option<Reg> {
        if self.is_avail(reg) { Some(reg) } else { None }
    }

    fn get_avail(&mut self) -> Option<Reg> {
        // Get the t-series first
        for i in 0..7 {
            if !self.t_reg[i] {
                self.t_reg[i] = true;
                return Some(Reg::T(i as u8));
            }
        }
        for i in 1..9 {
            if !self.a_reg[i % 8] {
                self.a_reg[i % 8] = true;
                return Some(Reg::A(i as u8));
            }
        }
        None
    }
}

impl NaiveRegAlloc {
    pub fn new() -> Self {
        Self {
            t_reg: [false; 7],
            a_reg: [false; 8],
        }
    }
}

impl Default for NaiveRegAlloc {
    fn default() -> Self {
        Self::new()
    }
}
