// 以下の ifdef ブロックは、DLL からのエクスポートを容易にするマクロを作成するための
// 一般的な方法です。この DLL 内のすべてのファイルは、コマンド ラインで定義された CDIFFER_EXPORTS
// シンボルを使用してコンパイルされます。このシンボルは、この DLL を使用するプロジェクトでは定義できません。
// ソースファイルがこのファイルを含んでいる他のプロジェクトは、
// CDIFFER_API 関数を DLL からインポートされたと見なすのに対し、この DLL は、このマクロで定義された
// シンボルをエクスポートされたと見なします。
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
