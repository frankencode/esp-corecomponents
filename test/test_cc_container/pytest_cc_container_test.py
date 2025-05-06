import pytest
from pytest_embedded_idf import IdfDut

def test_cc_container_test(dut: IdfDut) -> None:
    dut.run_all_single_board_cases()
