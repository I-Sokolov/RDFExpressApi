
extern void IFC4_test();
extern void AP242_test();
extern void HelloWall();
extern void GuideExamples();
extern void EngineTests(void);
extern void ADB_test();

extern int main()
{
    ADB_test();

    EngineTests();

    IFC4_test();

    AP242_test();

    HelloWall();

    GuideExamples();

    return 0;
}