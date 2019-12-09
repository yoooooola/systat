// 프로그램 실행 시
// who 커맨드 구현으로 현재 로그인된 pts들의 PID 가져오기
// 구한 PID로 /proc/PID/smaps 파일 복사본 만들기 ( PID_prev )
// 5초 뒤에 PID_cur 파일 복사본 만들기
// 7ffe367c2000-7ffe367c4000 -> 주소. 두개 주소앞에 0x 붙여서
// 아래 커맨드로 실행
// dump memory ./dump_outputfile.dump 0x2b3289290000 0x2b3289343000
// 비교한뒤 strings 나 hexdump -C 사용하여 dump 파일 표시하기
