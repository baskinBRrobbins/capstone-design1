void setup() {
  // put your setup code here, to run once:
  //가장 최근까지 수정한 코드
Serial.begin(9600);
}

void loop() {
 
  //압력센서 입력의 소속도함수, 1번입력
  float push_Big[3]={140, 500, 500};
  float push_Middle[3]={0, 140, 500};
  float push_Small[3]={0, 0, 140};
  //전류출력(모터상태)에 대한 소속도함수, 2번 입력
  float current_Big[3] = {101, 103, 103};
  float current_Middle[3] = {99, 101, 103};
  float current_Small[3] = {99, 99, 101};

  //입력들의 조합에대한 결과의 소속도함수//수업처럼 조합사용//모터상태랑 같은데??? 상관없을까?
  float out_B[3] = {101, 103, 103};
  float out_M[3] = {99, 101, 103};
  float out_S[3] = {99, 99, 101};
  //조합//3by3이므로 총9개의 규칙이 존재한다.
  //입력 조합,IF_then에 입력될값
  float* input_memb_info[]= {push_Big,current_Big,//포인터행렬로 들어감 괄호수 주의
                                push_Middle,current_Big,
                                push_Small,current_Big,
                                push_Big,current_Middle,
                                push_Middle,current_Middle,
                                push_Small,current_Middle,
                                push_Big,current_Small,
                                push_Middle,current_Small,
                                push_Small,current_Small};
  //출력쪽 소속도함수, defuzzifier에 입력될값
  float* out_memb_info[]= {out_B, out_M,out_S,
                              out_B,out_M,out_S,
                              out_B,out_M,out_S};

  //입력
  float* w;//벡터 출력이므로 포인터로 받음!!
  float x[] = {444,99.5};//여기에 압력센서값과 전류값을 읽어옴수정!!!!!!!!!
  w = IF_THEN(x,input_memb_info);
  Serial.print("\n");
  float u = defuzzifier(w, out_memb_info);//u는 최종출력값으로 모터를 돌릴 전류값이 나온다.
  Serial.print("u:");
 Serial.print(u);
 Serial.print("\n");

 delay(3000);
float s[] = {55,102.33};
  w = IF_THEN(s,input_memb_info);
  Serial.print("\n");
float a = defuzzifier(w, out_memb_info);//u는 최종출력값으로 모터를 돌릴 전류값이 나온다.
Serial.print("a:");
 Serial.print(a);
 Serial.print("\n");

 delay(3000);

}


float* IF_THEN(float x[],float* memb_info[]){ //memb_info의 가로크기는 지정,  배열을 return
  int p = 2;//입력의 수이므로 (2종류입력)
  int r = 9;//규칙의 수이므로 9개로 지정
  static float muA[2];
  static float w[9];//규칙의수
  for (int i=0;i<r;i++){ //(2종류입력)*(9개의 규칙)=18번 for문이 돈다.
    for (int j=0;j<p;j++){
      float mem_infos[3]={*(memb_info[i*p+j]),*(memb_info[i*p+j]+1),*(memb_info[i*p+j]+2)};
      muA[j] = membership(x[j],mem_infos); /////포인터는 0부터 시작 //*(memb_info[n번째입력]+함수의 범위값)
      
    }
    Serial.print("  muA[0]");Serial.print(muA[0]);Serial.print("  muA[1]");Serial.print(muA[1]);
    float min_muA = muA[0];
    for (int j=2;j<=p;j++){ //min(muA)를 수행하기위한 for문
      min_muA = min(min_muA,muA[j]);
      }
   w[i] = min_muA; Serial.print("  ");Serial.print(min_muA);
   Serial.print("\n");

  }
  return w;
  }
  ///////////////////////////////////////////
  float membership(float x, float memb_info[]){
  double mu;
  float xl = memb_info[0];
  float xm = memb_info[1];
  float xu = memb_info[2];
  if (xm == xl){
  // -↘_
  mu = (1/(-xu+xl))*(x-xu);
  }
  else if (xm == xu){
  // _↗-
  mu = (1/(xu-xl))*(x-xl);
  }
  else{
 // _↗↘_
    if (x < xm){
    mu = (1/(-xm+xl))*(-x+xl);
    }
    else{
    mu = (1/(-xu+xm))*(x-xu);
    }
  }
mu = max(min(mu,1),0);


return mu;
  }
  ////////////////////////////////////////////
float defuzzifier(float* w, float* memb_info[]){//여기의 memb_info는 if_then과는 다름//w는 포인터
  //여기서 memb_info의 크기는 규칙의 수와 같다.//9개 포인터이므로 사용에 주의
  float ybar[9];//크기는 규칙의 수와 같다.
  int r = 9;// 규칙의 수이므로//9
  
  for(int i=0;i<r;i++){//가중평균계산을 위한 평균값 for문//포인터이므로 0부터 시작
  ybar[i]=(*(memb_info[i])+ *(memb_info[i]+2))/2; //ybar또한 벡터로 평균저장 우선 2개의 규칙
  }
  
  float mub_star[r];//규칙의 결과 저장
  for (int i=0;i<r;i++){//i번째 규칙에서
    float memb_infos[3]={*(memb_info[i]),*(memb_info[i]+1),*(memb_info[i]+2)};//i번째 규칙의 소속도 함수 결정값
    mub_star[i] = min(w[i-1],membership(ybar[i],memb_infos)); //더 작은 값이 규칙의 결과로 입력된다//w는 포인터이므로i-1
    //Serial.print(" mub_stars:");Serial.print(mub_star[i]); Serial.print(" ");
  }
  float eps = 1e-6;
  static float ystar;
  float total_up=0;//단순히 합을 저장하기위한 변수
  float total_down=0;
  for(int i=0;i<r;i++){//규칙은 2개이상
    total_up = total_up + ybar[i]*mub_star[i];
    total_down = total_down + mub_star[i];   
  }
  ystar = (total_up + eps)/(total_down+eps);// crisp값 결정
  Serial.print("ystar: ");Serial.print(ystar);Serial.print("\n");
  return ystar;
}
