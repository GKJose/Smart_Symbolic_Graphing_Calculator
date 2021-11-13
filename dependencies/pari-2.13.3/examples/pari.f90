! Example program that shows how to create FORTRAN binding for PARI
module PARI
  use ISO_C_BINDING, only : C_LONG, C_DOUBLE, C_PTR
  interface
    subroutine pari_init(parisize, maxprime) bind(C,name='pari_init')
      import C_LONG
      integer(kind=C_LONG), VALUE              :: parisize
      integer(kind=C_LONG), VALUE              :: maxprime
    end subroutine pari_init
    !
    subroutine pari_close() bind(C,name='pari_close')
    end subroutine pari_close
    !
    type(C_PTR) function dbltor( r ) bind(C,name='dbltor')
      import C_DOUBLE, C_PTR
      real(kind=C_DOUBLE), VALUE  :: r
    end function dbltor
    !
    real(kind=C_DOUBLE) function rtodbl( x ) bind(C,name='rtodbl')
      import C_DOUBLE, C_PTR
      type(C_PTR), VALUE :: x
    end function rtodbl
    !
    type(C_PTR) function gsqr( x ) bind(C,name='gsqr')
      import C_PTR
      type(C_PTR), VALUE :: x
    end function gsqr
    !
    type(C_PTR) function gmul( x , y) bind(C,name='gmul')
      import C_PTR
      type(C_PTR), VALUE :: x
      type(C_PTR), VALUE :: y
    end function gmul
    !
    type(C_PTR) function gprec( x , d) bind(C,name='gprec')
      import C_PTR, C_LONG
      type(C_PTR), VALUE :: x
      integer(kind=C_LONG), VALUE :: d
    end function gprec
    !
    type(C_PTR) function gmod( x , y) bind(C,name='gmod')
      import C_PTR
      type(C_PTR), VALUE :: x
      type(C_PTR), VALUE :: y
    end function gmod
    !
    type(C_PTR) function glog( x , prec) bind(C,name='glog')
      import C_PTR, C_LONG
      type(C_PTR), VALUE :: x
      integer(kind=C_LONG), VALUE :: prec
    end function glog
    !
    type(C_PTR) function stoi(x) bind(C,name='stoi')
      import C_PTR, C_LONG
      integer(kind=C_LONG), VALUE :: x
    end function stoi
    !
    integer(kind=C_LONG) function itos(x) bind(C,name='itos')
      import C_PTR, C_LONG
      type(C_PTR), VALUE :: x
    end function itos
    !
    type(C_PTR) function Pi2n(x, prec) bind(C,name='Pi2n')
      import C_PTR, C_LONG
      integer(kind=C_LONG), VALUE :: x
      integer(kind=C_LONG), VALUE :: prec
    end function Pi2n
 end interface
end module PARI
!
PROGRAM prog
  use ISO_C_BINDING, only : C_PTR, C_DOUBLE
  use PARI
  implicit none
  real(kind=C_DOUBLE) :: r      = 1e36
  type(C_PTR)         :: p
  integer(kind=C_LONG) :: prec = 20 ! 18 words
  CALL pari_init(10000000_8,2_8)
  p = glog(stoi(10000_8),prec)
  p = gmod(p, Pi2n(1_8, prec))
  r = rtodbl(p)
  CALL pari_close()
  PRINT '(a,f0.9)','log(10000)%(2*Pi)  = ', r
END PROGRAM prog
