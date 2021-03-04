
#ifdef CDIFFER_EXPORTS
#define CDIFFER_API __declspec(dllexport)
#else
#define CDIFFER_API __declspec(dllimport)
#endif

// このクラスは dll からエクスポートされました
class CDIFFER_API Ccdiffer {
public:
	Ccdiffer(void);
	// TODO: メソッドをここに追加します。
};

extern CDIFFER_API int ncdiffer;

CDIFFER_API int fncdiffer(void);

