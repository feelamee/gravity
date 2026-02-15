skip -rfunction glm::vec.*

skip -rfunction Im(Vec2|Vec4|Strv|Vector|Span)::.+

define mr
	make
	run
end
define ms
	make
	start
end
