### DISCLAIMER: We will not use this project for Operating Systems and Lab (CS330) by Youngjin Kwon from 2025 Fall Semester.

Brand new pintos for Operating Systems and Lab (CS330), KAIST, by Youngjin Kwon.

The manual is available at https://casys-kaist.github.io/pintos-kaist/.

## 디렉토리 구조

### 핵심 디렉토리

📁 pintos-kaist  
├── 📁 devices : 프로젝트 1  
├── 📁 filesys : 프로젝트 2, 프로젝트 4    
├── 📁 include  
│   ├── 📁 devices  
│   ├── 📁 filesys  
│   ├── 📁 lib  
│   ├── 📁 threads  
│   ├── 📁 userprog  
│   ├── 📁 vm  
├── 📁 lib : 프로젝트 2  
│   ├── 📁 kernel  
│   ├── 📁 user  
├── 📁 tests  
│   ├── 📁 Algorithm  
│   ├── 📁 filesys  
│   ├── 📁 internal  
│   ├── 📁 threads  
│   ├── 📁 userprog  
│   ├── 📁 vm  
├── 📁 threads : 프로젝트 1  
├── 📁 userprog : 프로젝트 2  
├── 📁 utils  
├── 📁 vm : 프로젝트 3
  
- **`threads/`**: 프로젝트 1부터 수정하게 될 기본 커널의 소스 코드입니다.
- **`userprog/`**: 프로젝트 2부터 수정하게 될 사용자 프로그램 로더의 소스 코드입니다.
- **`vm/`**: 거의 비어 있는 디렉터리입니다. 프로젝트 3에서 가상 메모리를 구현하게 됩니다.
- **`filesys/`**: 기본 파일 시스템 소스 코드입니다. 프로젝트 2부터 이 파일 시스템을 사용하지만, 프로젝트 4까지는 수정하지 않습니다.
- **`devices/`**: 키보드, 타이머, 디스크 등의 I/O 장치 인터페이싱을 위한 소스 코드입니다. 프로젝트 1에서 타이머 구현을 수정합니다. 그렇지 않으면 이 코드를 변경할 필요가 없습니다.

### 라이브러리 및 헤더

- **`lib/`**: 표준 C 라이브러리의 하위 집합을 구현한 것입니다. 이 디렉토리의 코드는 Pintos 커널과 프로젝트 2부터 Pintos 커널에서 실행되는 사용자 프로그램 모두에 컴파일됩니다. 이 코드는 수정할 필요가 거의 없습니다.
- **`include/lib/kernel/`**: Pintos 커널에만 포함된 C 라이브러리의 일부입니다. 여기에는 커널 코드에서 자유롭게 사용할 수 있는 비트맵, 이중 연결 리스트, 해시 테이블 등 일부 데이터 유형의 구현도 포함됩니다. 커널에서 이 디렉터리의 헤더는 `#include <...>` 표기법을 사용하여 포함될 수 있습니다.
- **`include/lib/user/`**: Pintos 사용자 프로그램에만 포함되는 C 라이브러리의 일부입니다. 사용자 프로그램에서 이 디렉터리의 헤더는 `#include <...>` 표기법을 사용하여 포함될 수 있습니다.
- **`include/`**: 헤더 파일에 대한 소스 코드(`*.h`)입니다.

### 테스트 및 유틸리티

- **`tests/`**: 각 프로젝트에 대한 테스트입니다. 제출한 과제를 테스트하는 데 도움이 된다면 이 코드를 수정하실 수 있지만, 테스트를 실행하기 전에 원본 코드로 교체하겠습니다.
- **`utils/`**: 빌드 및 실행에 필요한 유틸리티 도구들입니다.

### 빌드 파일

- **`Makefile*`**: 프로젝트 빌드를 위한 Makefile들입니다.
- **`Make.config`**: 빌드 설정 파일입니다.
- **`install.sh`**: 설치 스크립트입니다.
- **`activate`**: 환경 활성화 스크립트입니다.

## 프로젝트 진행 순서

1. **프로젝트 1**: `threads/` 디렉토리의 기본 커널 및 `devices/` 디렉토리의 타이머 구현
2. **프로젝트 2**: `userprog/` 디렉토리의 사용자 프로그램 로더 및 `filesys/` 파일 시스템 사용
3. **프로젝트 3**: `vm/` 디렉토리의 가상 메모리 구현
4. **프로젝트 4**: `filesys/` 디렉토리의 파일 시스템 개선