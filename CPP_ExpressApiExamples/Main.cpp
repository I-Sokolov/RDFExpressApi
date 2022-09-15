
extern void IFC4_test();
extern void AP242_test();
extern void HelloWall();
extern void GuideExamples();
extern void EngineTests(void);

extern int main()
{
    EngineTests();

    IFC4_test();

    AP242_test();

    HelloWall();

    GuideExamples();
    
    return 0;
}