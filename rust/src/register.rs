#[derive(Copy, Clone, Debug, Eq, PartialEq, Hash)]
pub enum Reg {
    Zero,  // x0
    Ra,    // x1
    Sp,    // x2
    Gp,    // x3
    Tp,    // x4
    T(u8), // t0..t6
    A(u8), // a0..a7
    S(u8), // s0..s11 (s0 is also fp)
}

impl Reg {
    /// Convert ABI reg to architectural x-number (0..31).
    pub fn xnum(self) -> u8 {
        match self {
            Reg::Zero => 0,
            Reg::Ra => 1,
            Reg::Sp => 2,
            Reg::Gp => 3,
            Reg::Tp => 4,

            Reg::T(i) => {
                debug_assert!(i <= 6);
                if i <= 2 { 5 + i } else { 25 + i } // t0-2: x5-7, t3-6: x28-31
            }

            Reg::A(i) => {
                debug_assert!(i <= 7);
                10 + i // a0-7: x10-17
            }

            Reg::S(i) => {
                debug_assert!(i <= 11);
                if i <= 1 { 8 + i } else { 16 + i } // s0-1: x8-9, s2-11: x18-27
            }
        }
    }

    pub fn is_allocatable(self) -> bool {
        !matches!(self, Reg::Zero | Reg::Ra | Reg::Sp | Reg::Gp | Reg::Tp)
    }

    pub fn is_callee_saved(self) -> bool {
        matches!(self, Reg::S(_))
    }

    pub fn is_caller_saved(self) -> bool {
        self.is_allocatable() && !self.is_callee_saved()
    }
}

impl std::fmt::Display for Reg {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match *self {
            Reg::Zero => write!(f, "x0"), // Can be zero, but following the tutorial
            Reg::Ra => write!(f, "ra"),
            Reg::Sp => write!(f, "sp"),
            Reg::Gp => write!(f, "gp"),
            Reg::Tp => write!(f, "tp"),
            Reg::T(i) => write!(f, "t{}", i),
            Reg::A(i) => write!(f, "a{}", i),
            Reg::S(i) => write!(f, "s{}", i),
        }
    }
}
