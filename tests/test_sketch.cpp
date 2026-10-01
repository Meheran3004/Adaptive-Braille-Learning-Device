#include <cassert>
#include <iostream>
#include "../Adaptive_braille_project/Adaptive_braille_project.ino"

void tick(unsigned long elapsed){fakeNow+=elapsed; loop();}
void press(int pin){pins[pin]=LOW;tick(1);tick(DEBOUNCE_MS);pins[pin]=HIGH;tick(1);tick(DEBOUNCE_MS);}
bool outputsOff(){for(int pin:BRAILLE_PINS)if(pins[pin])return false;return true;}
void expectAudio(int f,int t){
  auto actual=dfPlayer.played.back();
  if(actual!=std::make_pair(f,t)){
    std::cerr<<"Expected audio "<<f<<","<<t<<"; got "<<actual.first<<","<<actual.second<<"\n";
    assert(false);
  }
}
void finishAudio(){tick(audioDuration + 1);}
void finishQuizWordPrompt(){
  finishAudio(); expectAudio(3,6); assert(phase==Phase::Ready);
  finishAudio(); assert(phase==Phase::Ready);
}
void startQuizLetter(){
  press(SPELL_BTN);
  assert(phase==Phase::Answering && outputsOff());
  finishAudio(); // Letter audio ends; no 006 prompt is played here.
  assert(phase==Phase::Answering);
}
void finishNextLetter(){
  assert(phase==Phase::Answering);
  finishAudio(); assert(phase==Phase::Answering);
}
int main(){
  std::fill(std::begin(pins),std::end(pins),HIGH);
  fileLines={"ab","c"};setup();
  assert(wordCount==2 && outputsOff() && !modeSelected);
  // Track-finished/error messages must not remain in the DFPlayer UART queue.
  dfPlayer.messages={{5, 5}, {DFPlayerError, 7}};
  serviceDfPlayer();assert(dfPlayer.messages.empty() && dfPlayer.activeMessage==-1);
  expectAudio(3,1); // Startup asks for a mode; early WORD is ignored.
  press(WORD_BTN);assert(phase==Phase::NoWord);
  finishAudio();press(WORD_BTN);assert(phase==Phase::NoWord);
  press(MODE_BTN);expectAudio(3,2);assert(modeSelected && mode==Mode::Learning);
  finishAudio();

  // Learning menu: WORD chooses Letter/Word Learning and SPELL confirms it.
  press(WORD_BTN);expectAudio(3,7);finishAudio();
  press(SPELL_BTN);assert(learningActivityConfirmed && learningActivity==LearningActivity::Letter);
  expectAudio(3,13);press(WORD_BTN);assert(phase==Phase::NoWord);finishAudio();
  nextRandom=0;press(WORD_BTN);expectAudio(1,1);assert(currentLearningLetter=='a' && pins[32]==HIGH);
  finishAudio();press(SPELL_BTN);expectAudio(1,1);assert(currentLearningLetter=='a' && pins[32]==HIGH);
  finishAudio();
  // WORD advances in alphabet order, repeats after Z, and SPELL never skips a letter.
  for(int i=1;i<26;++i){
    press(WORD_BTN);expectAudio(1,i+1);
    assert(currentLearningLetter=='a'+i);
    for(int dot=0;dot<6;++dot) assert(pins[BRAILLE_PINS[dot]]==((BRAILLE[i]>>dot)&1));
    finishAudio();
  }
  press(WORD_BTN);expectAudio(1,1);assert(currentLearningLetter=='a');finishAudio();

  // Re-enter Learning and select Word Learning for the existing manual-word flow.
  press(MODE_BTN);expectAudio(3,3);finishAudio();
  press(MODE_BTN);expectAudio(3,2);finishAudio();
  press(WORD_BTN);expectAudio(3,7);finishAudio();
  press(WORD_BTN);expectAudio(3,8);finishAudio();
  press(SPELL_BTN);assert(learningActivityConfirmed && learningActivity==LearningActivity::Word);
  expectAudio(3,13);finishAudio();
  press(WORD_BTN);expectAudio(2,1);finishAudio();
  press(SPELL_BTN);expectAudio(1,1);assert(pins[32]==HIGH && pins[33]==LOW);
  tick(5000);assert(letterIndex==1); // No timed advancement in learning.
  press(SPELL_BTN);expectAudio(1,2);assert(pins[33]==HIGH);finishAudio();
  press(SPELL_BTN);assert(phase==Phase::Complete && outputsOff());
  assert(wordList[0].weight==BASE_WEIGHT); // Learning does not alter weights.

  press(MODE_BTN);expectAudio(3,3);finishAudio();
  assert(mode==Mode::Quiz && phase==Phase::NoWord && outputsOff());
  // Select Word Quiz through its menu for the existing word regression tests.
  press(WORD_BTN);expectAudio(3,9);finishAudio();
  press(WORD_BTN);expectAudio(3,10);finishAudio();
  press(SPELL_BTN);assert(quizActivityConfirmed && quizActivity==QuizActivity::Word);
  expectAudio(3,13);finishAudio();
  nextRandom=0;press(WORD_BTN);expectAudio(2,1);finishQuizWordPrompt();
  wordList[0].weight=BASE_WEIGHT+2;
  press(SPELL_BTN);assert(phase==Phase::Answering);
  press(BRAILLE_INPUT_PINS[0]);press(SPELL_BTN);
  assert(guess==0 && letterIndex==0 && phase==Phase::Answering);
  finishAudio();assert(phase==Phase::Answering);

  // Bounce is ignored; holding a dot produces only one toggle.
  int dot1=BRAILLE_INPUT_PINS[0];
  pins[dot1]=LOW;tick(1);tick(10);pins[dot1]=HIGH;tick(1);tick(40);assert(guess==0);
  pins[dot1]=LOW;tick(1);tick(35);tick(200);assert(guess==1);
  pins[dot1]=HIGH;tick(1);tick(35);
  press(SPELL_BTN);expectAudio(1,2);
  assert(letterIndex==1 && guess==0 && outputsOff());
  assert(wordList[0].weight==BASE_WEIGHT+2); // No reward for only part of a word.
  finishNextLetter();
  press(dot1);press(BRAILLE_INPUT_PINS[1]);press(SPELL_BTN);
  expectAudio(3,4);assert(phase==Phase::Complete && wordList[0].weight==BASE_WEIGHT+1);
  finishAudio();press(SPELL_BTN);assert(wordList[0].weight==BASE_WEIGHT+1);

  // Force a draw for c, submit an empty (incorrect) answer.
  nextRandom=wordList[0].weight;
  press(WORD_BTN);expectAudio(2,2);finishQuizWordPrompt();startQuizLetter();
  press(SPELL_BTN);expectAudio(3,5);
  assert(phase==Phase::WrongFeedback && outputsOff());
  assert(wordList[1].weight==BASE_WEIGHT+1);
  press(SPELL_BTN);assert(phase==Phase::WrongFeedback);
  finishAudio();assert(phase==Phase::Correction && outputsOff());
  press(SPELL_BTN);expectAudio(1,3);assert(pins[32] && pins[26]);finishAudio();
  press(SPELL_BTN);assert(phase==Phase::Complete && outputsOff());
  assert(wordList[1].weight==BASE_WEIGHT+1); // Correction earns no reward.

  // Abandoning an answer and changing modes do not modify weights.
  nextRandom=0;press(WORD_BTN);finishQuizWordPrompt();startQuizLetter();press(dot1);
  assert(guess==1);press(MODE_BTN);expectAudio(3,2);
  assert(guess==0 && outputsOff() && phase==Phase::NoWord);
  assert(wordList[1].weight==BASE_WEIGHT+1);finishAudio();

  // Exact exhaustive distribution: weights 5 and 15 give 5 and 15 tickets.
  wordList[0].weight=5;wordList[1].weight=15;
  int hits[2]={0,0};
  mode=Mode::Quiz;
  for(nextRandom=0;nextRandom<20;++nextRandom) ++hits[selectWeightedIndex()];
  assert(hits[0]==5 && hits[1]==15 && wordForTicket(20)==-1);
  nextRandom=5;selectRandomWord();assert(currentWordIndex==1);
  selectRandomWord();assert(currentWordIndex==1); // Quiz repeats are allowed.

  // Floor, upper bound, and no changes for invalid or Learning selections.
  currentWordIndex=0;wordList[0].weight=BASE_WEIGHT;
  updateWordWeight(true);assert(wordList[0].weight==BASE_WEIGHT);
  wordList[0].weight=MAX_WEIGHT;
  updateWordWeight(false);assert(wordList[0].weight==MAX_WEIGHT);
  updateWordWeight(true);assert(wordList[0].weight==MAX_WEIGHT-WEIGHT_STEP);
  currentWordIndex=-1;updateWordWeight(false);
  mode=Mode::Learning;currentWordIndex=0;updateWordWeight(false);
  assert(wordList[0].weight==MAX_WEIGHT-WEIGHT_STEP);
  resetWeights();assert(wordList[0].weight==BASE_WEIGHT && wordList[1].weight==BASE_WEIGHT);

  // Hardware reset runs setup again and restores mode selection and all weights.
  wordList[1].weight=33;setup();
  assert(!modeSelected && phase==Phase::NoWord && currentWordIndex==-1);
  assert(wordList[0].weight==BASE_WEIGHT && wordList[1].weight==BASE_WEIGHT);
  expectAudio(3,1);

  // Reject invalid words without shifting the following audio track numbers.
  fileLines={" A ","bad-word","","B"};loadWordList();
  assert(wordCount==2 && wordList[1].audioNum==3 && wordList[1].weight==BASE_WEIGHT);
  wordCount=0;selectRandomWord();assert(phase==Phase::NoWord);
  mode=Mode::Quiz;assert(selectWeightedIndex()==-1);
  wordCount=1;wordList[0].word="a";wordList[0].audioNum=1;wordList[0].weight=BASE_WEIGHT;
  nextRandom=0;selectRandomWord();selectRandomWord();assert(currentWordIndex==0);

  // Maximum configured population/weights cannot overflow the draw.
  wordCount=MAX_WORDS;
  for(int i=0;i<wordCount;++i) wordList[i].weight=MAX_WEIGHT;
  nextRandom=MAX_WORDS*MAX_WEIGHT-1;
  assert(selectWeightedIndex()==MAX_WORDS-1);

  // MODE cancels an active letter recording.
  wordCount=1;currentWord="a";letterIndex=0;modeSelected=true;mode=Mode::Quiz;
  phase=Phase::Answering;announceLetter();
  press(MODE_BTN);expectAudio(3,2);finishAudio();tick(10000);
  expectAudio(3,2);assert(phase==Phase::NoWord);

  // Letter Quiz: weighted random draw, no answer revealed, one instruction,
  // correct reward, wrong penalty, manual correction, and independent weights.
  press(MODE_BTN);expectAudio(3,3);finishAudio();
  press(SPELL_BTN);assert(!quizActivityConfirmed);
  press(WORD_BTN);expectAudio(3,9);finishAudio();
  press(SPELL_BTN);assert(quizActivityConfirmed && quizActivity==QuizActivity::Letter);
  expectAudio(3,13);press(WORD_BTN);assert(phase==Phase::NoWord);finishAudio();
  resetWeights();letterWeights[0]=BASE_WEIGHT+2;
  int letterHits[26]={0};
  for(uint32_t ticket=0;ticket<132;++ticket) ++letterHits[letterForTicket(ticket)];
  assert(letterHits[0]==7 && letterForTicket(132)==-1);
  for(int i=1;i<26;++i) assert(letterHits[i]==5);
  nextRandom=0;press(WORD_BTN);expectAudio(1,1);assert(outputsOff());
  press(SPELL_BTN);assert(phase==Phase::QuizInstruction);
  finishAudio();expectAudio(3,6);assert(phase==Phase::Answering);
  press(dot1);assert(guess==0);finishAudio();
  press(dot1);press(SPELL_BTN);expectAudio(3,11);
  assert(phase==Phase::Complete && letterWeights[0]==6 && wordList[0].weight==BASE_WEIGHT);
  finishAudio();press(SPELL_BTN);assert(letterWeights[0]==6);
  press(WORD_BTN);expectAudio(1,1);finishAudio();finishAudio();
  press(SPELL_BTN);expectAudio(3,12);assert(letterWeights[0]==7 && outputsOff());
  finishAudio();assert(phase==Phase::Correction);
  press(SPELL_BTN);expectAudio(1,1);assert(pins[32] && letterWeights[0]==7);
  finishAudio();press(SPELL_BTN);assert(phase==Phase::Complete && outputsOff());
  letterWeights[0]=BASE_WEIGHT;updateQuizWeight(true);assert(letterWeights[0]==BASE_WEIGHT);
  letterWeights[0]=MAX_WEIGHT;updateQuizWeight(false);assert(letterWeights[0]==MAX_WEIGHT);
  press(MODE_BTN);assert(letterWeights[0]==MAX_WEIGHT && currentQuizLetterIndex==-1);
  setup();for(auto weight:letterWeights) assert(weight==BASE_WEIGHT);
  assert(!quizActivityConfirmed);

  // Millisecond timer wraparound must not break debounce or audio sequencing.
  Button testButton(23);pins[23]=HIGH;testButton.begin();
  pins[23]=LOW;assert(!testButton.pressed(~0UL-10));
  assert(testButton.pressed(30));assert(!testButton.pressed(100));
  audioStarted=UINT32_MAX-10;audioDuration=100;
  assert(audioPlaying(20));assert(!audioPlaying(200));

  const int expected[26]={1,3,9,25,17,11,27,19,10,26,5,7,13,29,21,15,31,23,14,30,37,39,58,45,61,53};
  for(int i=0;i<26;++i)assert(BRAILLE[i]==expected[i]);
  std::cout<<"PASS: adaptive probabilities, weight bounds/reset, audio sequence, manual spelling, quiz and debounce\n";
}
