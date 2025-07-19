vm_SRC = vm/vm.c          # 메인 api 프록시
vm_SRC += vm/uninit.c     # 초기화되지 않은 페이지
vm_SRC += vm/anon.c       # 익명 페이지
vm_SRC += vm/file.c       # 파일 매핑된 페이지
vm_SRC += vm/inspect.c    # 테스트 유틸리티
