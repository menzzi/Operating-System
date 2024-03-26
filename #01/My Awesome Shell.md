과제 01
---
### 문제 배경
- 쉘 프로그램 mash( My Awesome SHell)은 프롬프트로 "$"를 인쇄한 후 명령줄 입력을 기다린다.
  명령줄을 입력하면 프레임워크는 parse_command()함수를 사용하여 명령줄을 토큰화하고 프레임워크는 run_command()토큰을 사용하여 함수를 호출한다. run_command()에서 시작하여 다음 기능을 구현해야 한다.
- 현재 쉘은 사용자가 입력할 때까지 계속해서 입력 명령을 받고 처리합니다 exit. 이 경우 쉘 프로그램이 종료된다.

1. 외부 명령 실행 : 쉘이 명령을 받으면 위에 배경에서 설명한 대로 실행 파일을 실행해야 한다.
     - 외부 실행파일을 실행. ( 예 : ls, pwd, cp )
     - 어떤 이유로 지정된 실행 파일을 실행할 수 없는 경우 stderr 인쇄.
2. 작업 디렉토리 변경
     - cd : 쉘에서 기능이 구현되는 내장 명령 구현
       각 사용자는 자신의 홈 디렉토리 ~를 갖습니다. 실제 경로는 $HOME환경 변수에 정의되어 있습니다.
       > 힌트:chdir(), getenv("HOME")

       ```c
       int run_command(int nr_tokens, char * const tokens[]){
          if (strcmp(tokens[0], "exit") == 0) return 0; // exit 경우 쉘 프로그램 종료.

          if(strcmp(tokens[0],"cd") == 0){

		        if(nr_tokens == 1){ // cd 만 있는 경우는 cd ~ 와 같음.
			        chdir(getenv("HOME"));
		        }else if(strcmp(tokens[1],"~") == 0){
			        chdir(getenv("HOME"));
		        }else{
			        chdir(tokens[1]); // HOME 이외의 명령일 경우.
		        }
	        }

       }
       ```
3. 명령 기록 유지 : 명령 내역을 유지하는 기능
   - 사용자가 history를 프롬프트에 입력하면 다음 형식으로 명령 내역을 인쇄합니다. 쉘이 이 명령을 자체적으로 처리해야 하므로 이는 history 내장 명령이 됩니다.
     `fprintf(stderr, "%2d: %s", index, command);`
     > struct list_head history 사용
   - 사용자가 !를 사용하여 기록에서 n번째 명령을 실행할 수 있도록 허용합니다 `! <number>`

     ```c
      extern struct list_head history;
     
      int run_command(int nr_tokens, char * const tokens[]){
          if(strcmp(tokens[0],"history") == 0){
		        struct entry* p = NULL;
		        int index = 0;

		        list_for_each_entry_reverse(p,&history,list){ // history를 역순으로 출력(예전명령을 순서대로)
			        fprintf(stderr,"%2d: %s",index,p->command);
			        index += 1;
		        }

	    }else if(strcmp(tokens[0],"!") == 0){
		        struct entry* p = NULL;
		        int index = 0;
		        char find_command[MAX_COMMAND_LEN]; // ! number에서 number번째 명령어 저장
		        int token = atoi(tokens[1]);
		        list_for_each_entry_reverse(p,&history,list){
			        if(index == token)break;
			        index++;
		        }
		        strcpy(find_command,p->command); // p->command 를 find_command로 복사

		        char * new_tokens[MAX_NR_TOKENS]={NULL};
		        int num = 0;
		        if(parse_command(find_command,&num,new_tokens)==0)return -1; // 찾은 명령어를 토큰으로 분할하고 분할되지 않으면 종료
		        run_command(num,new_tokens); // 분할된 토큰을 실행
           }

       }
     ```
     








       
